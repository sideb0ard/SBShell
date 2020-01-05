#pragma once

#include <defjams.h>
#include <pattern_parser/ast.hpp>

class PatternFunction
{
  public:
    PatternFunction() = default;
    virtual ~PatternFunction() = default;
    virtual std::string String() const = 0;
    virtual void TransformPattern(
        std::array<std::vector<std::shared_ptr<MusicalEvent>>, PPBAR> &events,
        int loop_num) const = 0;
    bool active_{true};
};

class PatternEvery : public PatternFunction
{
  public:
    PatternEvery(int every_n, std::shared_ptr<PatternFunction> func)
        : every_n_{every_n}, func_{func}
    {
    }
    void TransformPattern(
        std::array<std::vector<std::shared_ptr<MusicalEvent>>, PPBAR> &events,
        int loop_num) const override;
    std::string String() const override;

  private:
    int every_n_;
    std::shared_ptr<PatternFunction> func_;
};

class PatternReverse : public PatternFunction
{
  public:
    PatternReverse() = default;
    void TransformPattern(
        std::array<std::vector<std::shared_ptr<MusicalEvent>>, PPBAR> &events,
        int loop_num) const override;
    std::string String() const override;
};

class PatternRotate : public PatternFunction
{
  public:
    PatternRotate(unsigned int direction, int num_sixteenth_steps)
        : direction_{direction}, num_sixteenth_steps_{num_sixteenth_steps} {};
    void TransformPattern(
        std::array<std::vector<std::shared_ptr<MusicalEvent>>, PPBAR> &events,
        int loop_num) const override;
    std::string String() const override;

  private:
    unsigned int direction_;
    int num_sixteenth_steps_;
};