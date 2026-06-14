#pragma once

#include <stdbool.h>

#include <array>
#include <memory>
#include <vector>

#include "envelope_generator.h"
#include "granular_engine.h"
#include "soundgenerator.h"
#include "stepper.h"

namespace SBAudio {

class GranularLooper : public SoundGenerator {
 public:
  GranularLooper(std::string filename, unsigned int loop_mode);
  GranularLooper();
  ~GranularLooper() override;
  StereoVal GenNext(mixer_timing_info tinfo) override;
  std::string Status() override;
  std::string Info() override;
  void Start() override;
  void Stop() override;
  void EventNotify(broadcast_event event, mixer_timing_info tinfo) override;
  void NoteOn(midi_event ev) override;
  void NoteOff(midi_event ev) override;
  void AllNotesOff() override;
  void SetParam(std::string name, double val) override;

  void AddBuffer(std::unique_ptr<FileBuffer> fb) override;
  void SetSubParam(int id, std::string name, double val) override;

  void Reset();

 public:
  bool started_{false};

  // Multiple source buffers: index 0 is primary (drives loop timing/patterns).
  // Additional buffers are used by shhh mode for cross-source selection.
  std::vector<std::unique_ptr<FileBuffer>> file_buffers_;

  SoundGrainType grain_type_{SoundGrainType::Sample};

  GranularEngine engine_;

  bool reverse_mode_{false};
  bool reverse_pending_{false};

  EnvelopeGenerator eg_;  // start/stop amp

  bool stop_count_pending_{false};
  int stop_len_{0};
  int stop_countr_{0};

  bool stop_pending_{false};  // allow eg to stop
  int degrade_by_{0};

  // shhh mode: per-grain cross-source selection based on RMS loudness.
  // 0=off, 1=quietest-both, 2=loudest-both, 3=quietest-L/loudest-R,
  // 4=loudest-L/quietest-R
  int shhh_mode_{0};
  int shhh_window_frames_{0};  // 0 = use grain_duration_frames

 public:
  void SetPitch(double pitch_ratio);
  void SetIncrSpeed(double speed);
  void SetGrainDuration(int dur);
  void SetGrainDensity(int gps);
  void SetGranularSpray(int spray_ms);
  void SetQuasiGrainFudge(int fudgefactor);
  void SetGrainOverlap(double overlap);
  void SetGrainEnvShape(int shape);
  void SetReverseMode(bool b);

  void SetLoopMode(unsigned int m);
  void SetLoopLen(double bars);
  void SetScramblePending();
  void SetStopPending(int loops);
  void SetStutterPending();
  void SetReversePending();
  void SetSpeedulatePending();
  void SetGatePending();
  void SetSlowdownPending();
  void SetRepeatPending();
  void SetStrobePending();
  void SetPitchRampPending();
  void SetOctaveJumpPending();
  void SetPitchStaircasePending();

  void LaunchGrain(mixer_timing_info tinfo);

  void SetGrainSlopePct(int percent);
  void SetDegradeBy(int degradation);

  void SetPidx(int val);
  void SetPOffset(int poffset);
  void SetPlooplen(int plooplen);
  void SetPinc(int pinc);

 private:
  // shhh helpers — only used when file_buffers_.size() > 1 and shhh_mode_ > 0
  FileBuffer* SelectShhhBuffer(bool want_loudest);
  static double ComputeBufferRMS(const std::vector<double>* buf, int num_ch,
                                 int start_idx, int window_frames);
};

}  // namespace SBAudio
