#pragma once

#include <stdbool.h>
#include <wchar.h>

#include <array>

#include <SequenceEngine.h>
#include <defjams.h>
#include <pattern_functions.hpp>
#include <pattern_parser/ast.hpp>
#include <pattern_parser/parser.hpp>
#include <pattern_parser/tokenizer.hpp>

class Process
{
  public:
    Process() = default;
    ~Process();
    std::string Status();
    void Start();
    void Stop();
    void EventNotify(mixer_timing_info);
    void SetDebug(bool b);
    void ParsePattern();
    void Update(ProcessType process_type, ProcessTimerType timer_type,
                float loop_len, std::string command,
                ProcessPatternTarget target_type,
                std::vector<std::string> targets, std::string pattern,
                std::vector<std::shared_ptr<PatternFunction>> funcz);
    void
    EvalPattern(std::shared_ptr<pattern_parser::PatternNode> const &pattern,
                int target_start, int target_end);
    void AppendPatternFunction(std::shared_ptr<PatternFunction> func);

  public:
    ProcessType process_type_{ProcessType::NO_PROCESS_TYPE};

    // Command Process Vars
    ProcessTimerType timer_type_{ProcessTimerType::NO_PROCESS_TIMER_TYPE};
    float loop_len_{1};
    std::string command_{};

    // Pattern Process Vars
    ProcessPatternTarget target_type_{
        ProcessPatternTarget::NO_PROCESS_PATTERN_TARGET};
    std::vector<std::string> targets_;

    std::string pattern_;

    bool active_;
    bool debug_;

  private:
    std::shared_ptr<pattern_parser::PatternNode> pattern_root_;
    std::array<std::vector<std::shared_ptr<MusicalEvent>>, PPBAR>
        pattern_events_;
    std::vector<std::shared_ptr<PatternFunction>> pattern_functions_;
    int loop_counter_;
};
