/***
 * Murxla: A Model-Based API Fuzzer for SMT solvers.
 *
 * This file is part of Murxla.
 *
 * Copyright (C) 2019-2022 by the authors listed in the AUTHORS file.
 *
 * See LICENSE for more information on using this software.
 */
#ifndef __MURXLA__SOLVER_OPTION_H
#define __MURXLA__SOLVER_OPTION_H

#include <memory>
#include <sstream>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "rng.hpp"

namespace murxla {

class SolverOption
{
 public:
  SolverOption(const std::string& name,
               const std::vector<std::string>& depends,
               const std::vector<std::string>& conflicts);
  virtual ~SolverOption() = default;

  virtual std::string pick_value(RNGenerator& rng) const = 0;

  const std::string& get_name() const;
  const std::unordered_set<std::string>& get_conflicts() const;
  const std::unordered_set<std::string>& get_depends() const;

  void add_conflict(const std::string& opt_name);
  void add_depends(const std::string& opt_name);

 private:
  std::string d_name;
  std::unordered_set<std::string> d_depends;
  std::unordered_set<std::string> d_conflicts;
};

class SolverOptionBool : public SolverOption
{
 public:
  SolverOptionBool(const std::string& name,
                   bool default_value,
                   const std::vector<std::string>& depends   = {},
                   const std::vector<std::string>& conflicts = {});
  ~SolverOptionBool() = default;
  std::string pick_value(RNGenerator& rng) const override;

 private:
  bool d_default;
};

template <typename T>
class SolverOptionNum : public SolverOption
{
 public:
  SolverOptionNum(const std::string& name,
                  T min,
                  T max,
                  T default_value,
                  const std::vector<std::string>& depends   = {},
                  const std::vector<std::string>& conflicts = {})
      : SolverOption(name, depends, conflicts),
        d_min(min),
        d_max(max),
        d_default(default_value){};
  ~SolverOptionNum() = default;

  std::string pick_value(RNGenerator& rng) const override
  {
    std::stringstream ss;
    ss << rng.pick<T>(d_min, d_max);
    return ss.str();
  }

 private:
  T d_min;
  T d_max;
  T d_default;
};

class SolverOptionList : public SolverOption
{
 public:
  SolverOptionList(const std::string& name,
                   const std::vector<std::string>& values,
                   const std::string& default_value,
                   const std::vector<std::string>& depends   = {},
                   const std::vector<std::string>& conflicts = {});
  ~SolverOptionList() = default;
  std::string pick_value(RNGenerator& rng) const override;

 private:
  std::vector<std::string> d_values;
  std::string d_default;
};

using SolverOptions =
    std::unordered_map<std::string, std::unique_ptr<SolverOption>>;

}  // namespace murxla

#endif
