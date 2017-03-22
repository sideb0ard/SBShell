#include <iostream>

#define LINK_PLATFORM_MACOSX 1

#include <ableton/Link.hpp>
#include <ableton/link/HostTimeFilter.hpp>
#include <ableton/platforms/stl/Clock.hpp>

#include "ableton_link_wrapper.h"
#include "defjams.h"
#include "mixer.h"
#include "sampler.h"

extern mixer *mixr;

struct AbletonLink {
    ableton::Link m_link;
    std::atomic<bool> m_running;

    double m_requested_tempo;
    double m_quantum;
    bool m_reset_beat_time;
    bool m_is_playing;
    int m_sample_time;
    double m_samples_per_midi_tick;
    double m_midi_ticks_per_ms;
    double m_loop_len_in_samples;
    double m_loop_len_in_ticks;

    std::chrono::microseconds m_output_latency;
    std::chrono::microseconds mTimeAtLastClick;

    ableton::link::HostTimeFilter<ableton::platforms::stl::Clock>
        m_host_time_filter;

    AbletonLink(double bpm)
        : m_link(bpm), m_running(true), m_requested_tempo{bpm}, m_quantum{4.},
          m_reset_beat_time{false}, m_is_playing{false}, m_sample_time{0},
          m_samples_per_midi_tick{(60.0 / bpm * SAMPLE_RATE) / PPQN},
          m_midi_ticks_per_ms{PPQN / ((60.0 / bpm) * 1000)},
          m_loop_len_in_samples{m_samples_per_midi_tick * PPL},
          m_loop_len_in_ticks{PPL}

    {
        m_link.setTempoCallback(update_bpm);
        m_link.enable(true);
    }
};

void update_bpm(double bpm)
{
    printf("Changing bpm to %.2f\n", bpm);
    // mixr->bpm = bpm;
    mixr->m_ableton_link->m_samples_per_midi_tick =
        (60.0 / bpm * SAMPLE_RATE) / PPQN;
    mixr->m_ableton_link->m_midi_ticks_per_ms = PPQN / ((60.0 / bpm) * 1000);
    mixr->m_ableton_link->m_loop_len_in_samples =
        mixr->m_ableton_link->m_samples_per_midi_tick * PPL;
    mixr->m_ableton_link->m_loop_len_in_ticks = PPL;
    for (int i = 0; i < mixr->soundgen_num; i++) {
        for (int j = 0; j < mixr->sound_generators[i]->envelopes_num; j++) {
            update_envelope_stream_bpm(mixr->sound_generators[i]->envelopes[j]);
        }
        if (mixr->sound_generators[i]->type == SAMPLER_TYPE) {
            sampler_resample_to_loop_size((SAMPLER *)mixr->sound_generators[i]);
        }
    }
}

// c++ wrapper for Ableton Link to call from sbsh
extern "C" {

AbletonLink *new_ableton_link(double bpm)
{
    std::cout << "New Ableton Link object!" << std::endl;
    return new AbletonLink(bpm);
}

LinkData link_get_timing_data(AbletonLink *l)
{
    auto timeline = l->m_link.captureAppTimeline();
    const auto time = l->m_link.clock().micros();

    LinkData data;
    data.num_peers = l->m_link.numPeers();
    data.quantum = l->m_quantum;
    data.tempo = timeline.tempo();
    data.beat = timeline.beatAtTime(time, l->m_quantum);
    data.phase = timeline.phaseAtTime(time, l->m_quantum);

    return data;
}

link_callback_timing_data link_get_callback_timing_data(AbletonLink *l, double quantum)
{
    link_callback_timing_data data;
    data.this_quantum = l->m_quantum;
    data.beat_at_time = 0;
    data.phase_this_sample = 0;
    data.phase_last_sample = 0;
}

double link_get_bpm(AbletonLink *l)
{
    auto timeline = l->m_link.captureAppTimeline();
    return timeline.tempo();
}

void link_update_from_main_callback(AbletonLink *l, uint64_t i_host_time)
{

    // const auto buffer_begin_at_output = host_time + l->m_output_latency;
    // std::cout << "HOSTTIME: " << host_time << std::endl;

    const auto host_time = std::chrono::microseconds(i_host_time);
    // std::cout << "HOSTTIME: " << i_host_time << " and as chrono " <<
    // host_time.count() << std::endl;

    auto timeline = l->m_link.captureAudioTimeline();

    if (l->m_reset_beat_time) {
        std::cout << "Resetting beat time" << std::endl;
        timeline.requestBeatAtTime(0, host_time, l->m_quantum);
        l->m_reset_beat_time = false;
    }

    if (l->m_requested_tempo > 0) {
        timeline.setTempo(l->m_requested_tempo, host_time);
        l->m_requested_tempo = 0;
    }

    l->m_link.commitAudioTimeline(timeline);

    l->m_sample_time++;

}

void link_set_bpm(AbletonLink *l, double bpm)
{
    l->m_requested_tempo = bpm;
}

void link_reset_beat_time(AbletonLink *l)
{
    l->m_reset_beat_time = true;
}

double link_get_beat_current_quantum(AbletonLink *l, uint64_t i_host_time)
{
    const auto host_time = std::chrono::microseconds(i_host_time);
    auto timeline = l->m_link.captureAudioTimeline();
    return timeline.beatAtTime(host_time, l->m_quantum);
}

bool link_is_start_of_sixteenth(AbletonLink *l, uint64_t host_time, int sample_number)
{
    auto timeline = l->m_link.captureAudioTimeline();
    const auto microsPerSample = 1e6 / (double) SAMPLE_RATE;

    const auto this_sample_time = std::chrono::microseconds(host_time) + std::chrono::microseconds(llround(sample_number * microsPerSample));
    const auto lastSampleHostTime = this_sample_time - std::chrono::microseconds(llround(microsPerSample));

    bool is_new_sixteenth = false;
    if (timeline.phaseAtTime(this_sample_time, 0.25) < timeline.phaseAtTime(lastSampleHostTime, 0.25)) {
        is_new_sixteenth = true;
        //printf("SAMPLEPHASETIMEDIFF %f %f == %f\n", timeline.phaseAtTime(this_sample_time, 0.25), timeline.phaseAtTime(lastSampleHostTime, 0.25), timeline.phaseAtTime(lastSampleHostTime, 0.25) - timeline.phaseAtTime(this_sample_time, 0.25));
    }

    return is_new_sixteenth;
}

int link_get_sample_time(AbletonLink *l) { return l->m_sample_time; }

int link_get_samples_per_midi_tick(AbletonLink *l)
{
    return l->m_samples_per_midi_tick;
}

int link_get_loop_len_in_samples(AbletonLink *l)
{
    return l->m_loop_len_in_samples;
}

uint64_t link_get_host_time(AbletonLink *l)
{
    const auto host_time =
        l->m_host_time_filter.sampleTimeToHostTime(l->m_sample_time);

    uint64_t itime = host_time.count();

    return itime;
}

void temp_use_link_metronome(AbletonLink *l, void *outputBuffer, int numSamples)
{
    auto timeline = l->m_link.captureAudioTimeline();
    const auto beginHostTime =
        l->m_host_time_filter.sampleTimeToHostTime(l->m_sample_time);
	using namespace std::chrono;

    auto mBuffer = (float*) outputBuffer;

	auto quantum = 4;

  // Metronome frequencies
  static const double highTone = 1567.98;
  static const double lowTone = 1108.73;
  // 100ms click duration
  static const auto clickDuration = duration<double>{0.1};

  // The number of microseconds that elapse between samples
  const auto microsPerSample = 1e6 / SAMPLE_RATE;

  for (std::size_t i = 0; i < numSamples; ++i)
  {
    double amplitude = 0.;
    // Compute the host time for this sample and the last.
    const auto hostTime = beginHostTime + microseconds(llround(i * microsPerSample));
    const auto lastSampleHostTime = hostTime - microseconds(llround(microsPerSample));

    // Only make sound for positive beat magnitudes. Negative beat
    // magnitudes are count-in beats.
    if (timeline.beatAtTime(hostTime, quantum) >= 0.)
    {
      // If the phase wraps around between the last sample and the
      // current one with respect to a 1 beat quantum, then a click
      // should occur.
      if (timeline.phaseAtTime(hostTime, 1) < timeline.phaseAtTime(lastSampleHostTime, 1))
      {
        //std::cout << "NEW BEAT!" << std::endl;
        l->mTimeAtLastClick = hostTime;
      }

      const auto secondsAfterClick =
        duration_cast<duration<double>>(hostTime - l->mTimeAtLastClick);

      // If we're within the click duration of the last beat, render
      // the click tone into this sample
      if (secondsAfterClick < clickDuration)
      {
 		// If the phase of the last beat with respect to the current
 		       // quantum was zero, then it was at a quantum boundary and we
 		       // want to use the high tone. For other beats within the
 		       // quantum, use the low tone.
 		       const auto freq =
 		         floor(timeline.phaseAtTime(hostTime, quantum)) == 0 ? highTone : lowTone;

 		       // Simple cosine synth
 		       amplitude = cos(2 * M_PI * secondsAfterClick.count() * freq)
 		                   * (1 - sin(5 * M_PI * secondsAfterClick.count()));
 	  }
    }
    mBuffer[i] = amplitude;
    }
}

} // extern "C"
