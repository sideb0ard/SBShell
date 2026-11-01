#ifndef MIXER_H
#define MIXER_H

#include <portaudio.h>
#include <portmidi.h>
#include <pthread.h>

#include <ableton/Link.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <shared_mutex>
#include <unordered_map>

// Forward declarations to avoid namespace issues
namespace ableton {
class Link;
}
#include <filesystem>
#include <process.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "audio_action_queue.h"
#include "defjams.h"
#include "dsp/circular_buffer.h"
#include "fmsynth.h"
#include "fx/fx.h"
#include "soundgenerator.h"
#include "subsynth.h"
#include "websocket/web_socket_server.h"
#include "xfader.h"

struct PreviewBuffer {
  std::string filename{};
  std::vector<double> audio_buffer{};
  int num_channels{};
  int audio_buffer_len{};
  int audio_buffer_read_idx{};
  bool enabled{};

  StereoVal Generate();
};

struct file_monitor {
  std::string function_file_filepath;
  std::filesystem::file_time_type function_file_filepath_last_write_time;
  bool initialized{false};
  bool init_called{false};  // Track if init() was actually executed
};

struct Action {
  Action(double start_val, double end_val, int time_taken_ticks,
         std::string action_to_take);
  ~Action() = default;

  void Run();
  bool IsActive() {
    return active_;
  }

  enum class InterpolationType { Linear, Logarithmic };

  double start_val_{0};
  double end_val_{0};
  double cur_val_{0};
  double incr_{0};
  int dir_{1};  // 1 going forward, 0, going downard
  int time_taken_ticks_{0};
  int elapsed_ticks_{0};
  bool has_started_{false};
  bool active_{false};
  std::string action_to_take_{""};
  InterpolationType interpolation_type_{InterpolationType::Linear};
};

struct Mixer {
 public:
  Mixer();  // WebSocket server temporarily disabled

  PreviewBuffer preview;

  // for importing functions - monitor these files for changes
  std::vector<file_monitor> file_monitors;

  std::array<std::shared_ptr<Process>, MAX_NUM_PROC> processes_ = {};
  bool proc_initialized_{false};

  std::atomic<int> sound_generators_idx_{0};
  std::atomic<int> midi_tick_{
      0};  // updated each MidiTick, readable by any thread
  std::array<std::unique_ptr<SBAudio::SoundGenerator>, MAX_NUM_SOUND_GENERATORS>
      sound_generators_ = {};
  mutable std::shared_mutex
      sound_generators_mutex_;  // Protects sound_generators_ array (read-write
                                // lock)
  // Recent output of each sound generator; back() is the current sample.
  // Capacity is the FFT window size so Spectrum() can read a full window.
  static constexpr size_t kFftSize = 512;
  std::array<CircularBuffer<StereoVal, kFftSize>, MAX_NUM_SOUND_GENERATORS>
      soundgen_cur_val_{};

  // Per-generator gain for solo fade-in/out. Ramps toward solo_gain_target_
  // each sample to avoid clicks when soloing/unsoloing.
  static constexpr double kSoloRampRate = 1.0 / 512.0;  // ~11ms at 44100Hz
  std::array<double, MAX_NUM_SOUND_GENERATORS> solo_gain_;
  std::array<double, MAX_NUM_SOUND_GENERATORS> solo_gain_target_;

  // Cached status info - updated by audio thread, read by UI thread
  struct CachedFxStatus {
    std::string status;
    bool enabled{false};
  };
  struct CachedSgStatus {
    std::string info;    // from sg->Info()
    std::string status;  // from sg->Status()
    bool active{false};
    double volume{0};
    int effects_num{0};
    std::array<CachedFxStatus, kMaxNumSoundGenFx> fx;
    std::array<double, kMixerNumSendFx> mixer_sends{};
  };
  std::array<CachedSgStatus, MAX_NUM_SOUND_GENERATORS> cached_sg_status_{};
  std::atomic<int> cached_sg_count_{0};
  mutable std::mutex cached_status_mutex_;  // Light lock for string copies
  void UpdateCachedStatus();                // Called from audio thread

  std::array<std::shared_ptr<Fx>, kMixerNumSendFx> fx_;

  std::vector<std::unique_ptr<AudioActionItem>> delayed_action_items_ = {};

  XFader xfader_;

  std::mutex scheduled_actions_mutex_;
  std::vector<Action> running_actions_{};
  std::multimap<int, Action> scheduled_actions_{};

  std::vector<int> soloed_sound_generator_idz{};

  bool debug_mode{false};

  bool websocket_enabled_{false};

  double bpm{140};
  double bpm_to_be_updated{0};

  mixer_timing_info timing_info = {};
  std::chrono::microseconds mTimeAtLastClick{0};

  double volume{1};
  double volume_smooth_{
      1.0};  // ramps toward volume each sample to de-click hard cuts

  PortMidiStream *midi_stream;
  bool have_midi_controller{false};
  std::string midi_controller_name{};
  int midi_target{-1};
  // std::vector<int> midi_targets{};  // sound_generators_ idx
  bool midi_recording = {false};
  bool midi_loop_{false};
  bool midi_print = {false};
  int record_bars_{2};
  int term_rows_{0};  // cached terminal height for status bar placement
  std::atomic<float> dsp_load_{
      0.0f};  // audio callback load 0..100%, EMA smoothed
  double global_reverb_send_{0.0};
  double global_delay_send_{0.0};
  double global_eq_send_{0.0};
  double global_reverb_feedback_{0.0};
  double global_delay_feedback_{0.0};
  double global_eq_feedback_{0.0};
  StereoVal last_reverb_out_{};
  StereoVal last_delay_out_{};
  StereoVal last_eq_out_{};
  // Per-generator flag so a NaN-producing instrument is reported once, not
  // every sample.
  std::array<bool, MAX_NUM_SOUND_GENERATORS> soundgen_nan_reported_{};
  std::unordered_map<int, int>
      pending_note_ons_;  // note -> abs pos in multi-bar buffer
  std::unordered_map<int, std::string> midi_mapped_controls_ = {};

  WebsocketServer *websocket_server_{nullptr};

  // On-demand snapshot of a generator's last kFftSize samples. A caller
  // thread sets snapshot_request_sg_; the audio thread copies into snapshot_
  // and sets snapshot_ready_. See SnapshotVals().
  std::atomic<int> snapshot_request_sg_{-1};  // -1 = no pending request
  std::atomic<bool> snapshot_ready_{false};
  std::array<StereoVal, kFftSize> snapshot_{};
  std::mutex snapshot_caller_mutex_;
  std::vector<StereoVal> SnapshotVals(int sg_num);
  // dB magnitudes, kFftSize/2 + 1 bins; bin k = k * sample_rate / kFftSize Hz.
  // Empty if audio isn't running. Don't call from the audio thread.
  // Values are dBFS: 0 dB = a full-scale sine.
  std::vector<double> Spectrum(int sg_num);
  // Spectrum condensed to num_bands log-spaced bands, each 0.0-1.0 (-60 dB
  // and below -> 0, 0 dB -> 1). All zeros if audio isn't running.
  std::vector<double> SpectrumBands(int sg_num, int num_bands);

  void AddMidiMapping(int id, std::string param);
  void ResetMidiMappings();
  void PrintMidiMappings();
  void HandleMidiControlMessage(int data1, int data2);

  void AssignSoundGeneratorToMidiController(int soundgen_id);
  void RecordMidiToggle();
  void MidiLoopToggle();
  void MidiStop();
  void PrintMidiToggle();
  void ResetMidiRecording();
  void SetRecordBars(int bars);
  void PlaybackMidiLoopTick();
  void QuantizeMidiRecording(int subdivisions = 16);
  void EmitTimingDisplay();

  void CheckForDelayedEvents();
  void CheckForExternalMidiEvents();

  void ScheduleAction(int when, Action item);
  void RunScheduledActions();

  void Ps(bool all);

  std::string StatusEnv();
  std::string StatusMixr();
  std::string StatusProcz(bool all = false);
  std::string StatusSgz(bool all);

  void PrintRecordingBuffer();
  // Returns bar 0 for backwards-compat with midi_dump / MidiArray
  const MultiEventMidiPattern &RecordingBuffer() const {
    return recording_buffer_[0];
  }
  const std::vector<MultiEventMidiPattern> &RecordingBufferAll() const {
    return recording_buffer_;
  }
  std::vector<MultiEventMidiPattern> recording_buffer_{
      2};  // must match record_bars_

  void UpdateBpm(int bpm);
  void UpdateTimeUnit(unsigned int time_type, int val);
  void MidiTick();
  void EmitEvent(broadcast_event event);
  bool DelSoundgen(int soundgen_num);

  void PreviewAudio(std::unique_ptr<AudioActionItem> action);

  void PrintTimingInfo();
  void PrintDxAlgos();
  void PrintDxRatioz();
  void PrintFuncAndGenInfo();

  void AddSoundGenerator(std::unique_ptr<SBAudio::SoundGenerator> sg);

  void VolChange(float vol);
  void VolChange(int sig, float vol);
  void PanChange(int sig, float vol);

  StereoVal GetLastVal(int sg_num) {
    return soundgen_cur_val_[sg_num].back();
  }

  void UpdateTimingInfo(long long int frame_time);

  // Template function implementation must be in header
  static constexpr auto MIDI_TICK_FRAC_OF_BEAT = 1. / 960;

  template <typename SessionState>
  int GenNext(float *out, int frames_per_buffer, SessionState &sessionState,
              const double quantum,
              const std::chrono::microseconds beginHostTime) {
    using namespace std::chrono;

    // The number of microseconds that elapse between samples
    constexpr auto microsPerSample = 1e6 / SAMPLE_RATE;

    int return_bpm = 0;
    if (bpm_to_be_updated > 0) {
      return_bpm = bpm_to_be_updated;
      bpm_to_be_updated = 0;
    }
    xfader_.Update(timing_info);

    for (int i = 0, j = 0; i < frames_per_buffer; i++, j += 2) {
      double output_left = 0.0;
      double output_right = 0.0;

      timing_info.cur_sample++;

      if (preview.enabled) {
        StereoVal preview_audio = preview.Generate();
        output_left += preview_audio.left * 0.6;
        output_right += preview_audio.right * 0.6;
      }

      const auto hostTime =
          beginHostTime +
          microseconds(llround(static_cast<double>(i) * microsPerSample));

      timing_info.is_midi_tick = false;
      auto beat_time = sessionState.beatAtTime(hostTime, quantum);
      if (beat_time >= 0.) {
        if (beat_time >= timing_info.time_of_next_midi_tick) {
          timing_info.time_of_next_midi_tick =
              (double)((int)beat_time) +
              ((timing_info.midi_tick % PPQN) * Mixer::MIDI_TICK_FRAC_OF_BEAT);
          timing_info.is_midi_tick = true;
          MidiTick();
          timing_info.midi_tick++;
        }
      }

      StereoVal fx_delay_send{};
      StereoVal fx_reverb_send{};
      StereoVal fx_eq_send{};

      // Try shared lock for sound generator access (allows multiple readers).
      // If we can't get the lock (writer is active), use cached values to avoid
      // glitches
      if (sound_generators_mutex_.try_lock_shared()) {
        // Cache the size to avoid race conditions with other threads modifying
        // it
        int safe_generators_count =
            (sound_generators_idx_.load() <
             static_cast<int>(sound_generators_.size()))
                ? sound_generators_idx_.load()
                : static_cast<int>(sound_generators_.size());

        for (int k = 0; k < safe_generators_count; k++) {
          // Additional bounds check
          if (k >= sound_generators_.size()) break;

          auto &sg = sound_generators_[k];
          if (!sg) {
            soundgen_cur_val_[k].push_back(StereoVal{});
            continue;
          }
          StereoVal v = sg->GenNext(timing_info);
          // A NaN/Inf from one instrument would poison the whole mix (and
          // the global delay/reverb feedback loops), silencing everything.
          // Mute just that instrument and report it once.
          if (!std::isfinite(v.left) || !std::isfinite(v.right)) {
            v = StereoVal{};
            if (!soundgen_nan_reported_[k]) {
              soundgen_nan_reported_[k] = true;
              std::cerr << "\n[mixer] sound generator " << k
                        << " output NaN/Inf - muting it. ("
                        << sg->Info().substr(0, 60) << ")\n";
            }
          } else {
            soundgen_nan_reported_[k] = false;
          }
          soundgen_cur_val_[k].push_back(v);
        }
        sound_generators_mutex_.unlock_shared();
        if (snapshot_request_sg_.load(std::memory_order_relaxed) >= 0) {
          int req =
              snapshot_request_sg_.exchange(-1, std::memory_order_acq_rel);
          if (req >= 0) {
            size_t got =
                soundgen_cur_val_[req].copy_last(snapshot_.data(), kFftSize);
            // Zero the rest if this generator hasn't produced a full window
            std::fill(snapshot_.begin() + got, snapshot_.end(), StereoVal{});
            snapshot_ready_.store(true, std::memory_order_release);
          }
        }
      }
      // If we couldn't lock, soundgen_cur_val_ still has cached values from
      // last iteration

      // Now collect the FX send values while we still have the lock or can get
      // it We need to access sound_generators_[k] here, so we need protection
      std::array<std::array<double, kMixerNumSendFx>, MAX_NUM_SOUND_GENERATORS>
          fx_send_cache{};
      if (sound_generators_mutex_.try_lock_shared()) {
        int safe_count = (sound_generators_idx_.load() <
                          static_cast<int>(sound_generators_.size()))
                             ? sound_generators_idx_.load()
                             : static_cast<int>(sound_generators_.size());
        for (int k = 0; k < safe_count; k++) {
          if (sound_generators_[k]) {
            fx_send_cache[k] = sound_generators_[k]->mixer_fx_send_intensity_;
          }
        }
        sound_generators_mutex_.unlock_shared();
      }

      // Now process the cached values (outside the lock)
      int safe_generators_count =
          (sound_generators_idx_.load() <
           static_cast<int>(sound_generators_.size()))
              ? sound_generators_idx_.load()
              : static_cast<int>(sound_generators_.size());

      for (int k = 0; k < safe_generators_count; k++) {
        // Ramp solo gain toward target to avoid clicks on solo/unsolo
        double &gain = solo_gain_[k];
        const double target = solo_gain_target_[k];
        if (gain < target)
          gain = std::min(gain + kSoloRampRate, target);
        else if (gain > target)
          gain = std::max(gain - kSoloRampRate, target);

        if (gain > 0.0) {
          StereoVal cur = soundgen_cur_val_[k].back();
          output_left += cur.left * xfader_.GetValueFor(k) * gain;
          output_right += cur.right * xfader_.GetValueFor(k) * gain;

          fx_delay_send += cur * fx_send_cache[k][0] * gain;
          fx_reverb_send += cur * fx_send_cache[k][1] * gain;
          fx_eq_send += cur * fx_send_cache[k][2] * gain;
        }
      }

      // Global sends: dry mix + one-sample feedback from previous FX output
      fx_reverb_send.left += output_left * global_reverb_send_ +
                             last_reverb_out_.left * global_reverb_feedback_;
      fx_reverb_send.right += output_right * global_reverb_send_ +
                              last_reverb_out_.right * global_reverb_feedback_;
      fx_delay_send.left += output_left * global_delay_send_ +
                            last_delay_out_.left * global_delay_feedback_;
      fx_delay_send.right += output_right * global_delay_send_ +
                             last_delay_out_.right * global_delay_feedback_;
      fx_eq_send.left += output_left * global_eq_send_ +
                         last_eq_out_.left * global_eq_feedback_;
      fx_eq_send.right += output_right * global_eq_send_ +
                          last_eq_out_.right * global_eq_feedback_;

      auto delay_val = fx_[0]->Process(fx_delay_send);
      auto reverb_val = fx_[1]->Process(fx_reverb_send);
      auto eq_val = fx_[2]->Process(fx_eq_send);
      // Clamp before storing — last_*_out_ feeds back into fx sends next
      // sample. Unclamped growth (e.g. delay fb + gdlfb combining) causes NaN
      // which bypasses the output clamp and silences one channel.
      // NaN-safe: std::min/max with NaN return the non-NaN bound, which would
      // inject full-scale DC, so zero non-finite values first.
      auto clamp1 = [](double v) {
        if (!std::isfinite(v)) return 0.0;
        return std::max(-1.0, std::min(1.0, v));
      };
      auto sanitize = [](StereoVal &sv) {
        if (!std::isfinite(sv.left)) sv.left = 0.0;
        if (!std::isfinite(sv.right)) sv.right = 0.0;
      };
      sanitize(delay_val);
      sanitize(reverb_val);
      sanitize(eq_val);
      last_delay_out_ = {clamp1(delay_val.left), clamp1(delay_val.right)};
      last_reverb_out_ = {clamp1(reverb_val.left), clamp1(reverb_val.right)};
      last_eq_out_ = {clamp1(eq_val.left), clamp1(eq_val.right)};
      // EQ is an insert: reduce dry by send amount so geq=1 fully replaces dry.
      // Delay and reverb remain parallel sends (dry is unaffected by their
      // send).
      output_left = output_left * (1.0 - global_eq_send_) + delay_val.left +
                    reverb_val.left + eq_val.left;
      output_right = output_right * (1.0 - global_eq_send_) + delay_val.right +
                     reverb_val.right + eq_val.right;

      // Apply volume and clamp to safe range to prevent speaker damage
      if (volume_smooth_ < volume)
        volume_smooth_ = std::min(volume_smooth_ + kSoloRampRate, volume);
      else if (volume_smooth_ > volume)
        volume_smooth_ = std::max(volume_smooth_ - kSoloRampRate, volume);

      double final_left = volume_smooth_ * output_left;
      double final_right = volume_smooth_ * output_right;

      // Hard clamp to [-1.0, 1.0] to protect speakers/ears. clamp1 also
      // zeroes NaN/Inf, which plain >/< comparisons let straight through.
      final_left = clamp1(final_left);
      final_right = clamp1(final_right);

      out[j] = final_left;
      out[j + 1] = final_right;
    }

    if (websocket_enabled_ && websocket_server_) {
      websocket_server_->sendData(out, 2 * frames_per_buffer * sizeof(float));
    }

    return return_bpm;
  }

  bool IsValidProcess(int proc_num);
  bool IsValidSoundgenNum(int soundgen_num);
  bool IsValidFx(int soundgen_num, int fx_num);

  void Now();  // time now in midi ticks
  double GetHzPerBar();
  double GetHzPerTimingUnit(unsigned int timing_unit);
  int GetTicksPerCycleUnit(unsigned int event_type);
  void CheckForAudioActionQueueMessages();
  void ProcessActionMessage(std::unique_ptr<AudioActionItem> action);

  void AddFileToMonitor(std::string filepath);

  // for sending websocket to p5.js
  void SetWebsocketServer(WebsocketServer *ws) {
    websocket_server_ = ws;
  }
  void EnableWebSocket(bool en) {
    websocket_enabled_ = en;
  }
};

#endif  // MIXER_H
