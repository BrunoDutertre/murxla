#ifndef __SMTMBT__SOLVER_H
#define __SMTMBT__SOLVER_H

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "op.hpp"
#include "sort.hpp"
#include "util.hpp"

/* -------------------------------------------------------------------------- */

#define SMTMBT_MK_TERM_N_ARGS -1
#define SMTMBT_MK_TERM_N_ARGS_MIN 2
#define SMTMBT_MK_TERM_N_ARGS_MAX 11

/* -------------------------------------------------------------------------- */

namespace smtmbt {
class FSM;

/* -------------------------------------------------------------------------- */
/* Sort                                                                       */
/* -------------------------------------------------------------------------- */

class AbsSort;

using Sort = std::shared_ptr<AbsSort>;

class AbsSort
{
 public:
  virtual ~AbsSort(){};
  virtual size_t hash() const                                      = 0;
  virtual bool equals(const std::shared_ptr<AbsSort>& other) const = 0;

  virtual bool is_bool() const         = 0;
  virtual bool is_bv() const           = 0;
  virtual uint32_t get_bv_size() const = 0;

  void set_id(uint64_t id);
  uint64_t get_id() const;

  void set_kind(SortKind sort_kind);
  SortKind get_kind();

  void set_sorts(const std::vector<Sort>& sorts);
  const std::vector<Sort>& get_sorts() const;

 protected:
  uint64_t d_id   = 0u;
  SortKind d_kind = SORT_ANY;

 private:
  std::vector<Sort> d_sorts;
};

bool operator==(const Sort& a, const Sort& b);

std::ostream& operator<<(std::ostream& out, const Sort s);

struct HashSort
{
  std::size_t operator()(const Sort s) const;
};

/* -------------------------------------------------------------------------- */
/* Term                                                                       */
/* -------------------------------------------------------------------------- */

class AbsTerm
{
 public:
  AbsTerm(){};
  virtual ~AbsTerm(){};
  virtual size_t hash() const                                      = 0;
  virtual bool equals(const std::shared_ptr<AbsTerm>& other) const = 0;

  void set_id(uint64_t id);
  uint64_t get_id() const;

  void set_sort(Sort sort);
  Sort get_sort() const;

 protected:
  uint64_t d_id = 0u;
  Sort d_sort = nullptr;
};

using Term = std::shared_ptr<AbsTerm>;

bool operator==(const Term& a, const Term& b);

std::ostream& operator<<(std::ostream& out, const Term t);
std::ostream& operator<<(std::ostream& out, const std::vector<Term>& vector);

struct HashTerm
{
  size_t operator()(const Term t) const;
};

/* -------------------------------------------------------------------------- */
/* Solver                                                                     */
/* -------------------------------------------------------------------------- */

class Solver
{
 public:
  enum Result
  {
    UNKNOWN,
    SAT,
    UNSAT,
  };

  enum Base
  {
    BIN = 2,
    DEC = 10,
    HEX = 16,
  };

  enum SpecialValueBV
  {
    ZERO,
    ONE,
    ONES,
    MIN_SIGNED,
    MAX_SIGNED,
  };

  Solver(RNGenerator& rng);
  Solver() = delete;
  ~Solver() = default;

  virtual void new_solver() = 0;
  virtual void delete_solver() = 0;
  virtual bool is_initialized() const = 0;

  virtual TheoryIdVector get_supported_theories() const;
  virtual OpKindSet get_supported_op_kinds() const;
  virtual OpKindSet get_unsupported_op_kinds() const;
  virtual void configure_fsm(FSM* fsm) const;

  virtual Term mk_var(Sort sort, const std::string name) const   = 0;
  virtual Term mk_const(Sort sort, const std::string name) const = 0;
  virtual Term mk_fun(Sort sort, const std::string name) const   = 0;

  virtual Term mk_value(Sort sort, bool value) const                   = 0;
  virtual Term mk_value(Sort sort, uint64_t value) const               = 0;
  virtual Term mk_value(Sort sort, std::string value, Base base) const = 0;

  virtual Sort mk_sort(const std::string name, uint32_t arity) const        = 0;
  virtual Sort mk_sort(SortKind kind) const                                 = 0;
  virtual Sort mk_sort(SortKind kind, uint32_t size) const                  = 0;
  virtual Sort mk_sort(SortKind kind, const std::vector<Sort>& sorts) const = 0;

  virtual Term mk_term(const OpKind& kind,
                       std::vector<Term>& args,
                       std::vector<uint32_t>& params) const = 0;

  virtual Sort get_sort(Term term) const = 0;

  virtual void assert_formula(const Term& t) const = 0;

  virtual Result check_sat() const = 0;
  virtual Result check_sat_assuming(std::vector<Term>& assumptions) const = 0;

  virtual std::vector<Term> get_unsat_assumptions() const = 0;

  virtual void push(uint32_t n_levels) const = 0;
  virtual void pop(uint32_t n_levels) const  = 0;

  virtual void print_model() const = 0;

  virtual void reset_assertions() const = 0;

  const std::vector<Base>& get_bases() const;
  const std::vector<SpecialValueBV>& get_special_values_bv() const;

  virtual void set_opt(const std::string& opt,
                       const std::string& value) const = 0;

  virtual std::string get_option_name_incremental() const               = 0;
  virtual std::string get_option_name_model_gen() const                 = 0;
  virtual std::string get_option_name_unsat_assumptions() const         = 0;
  virtual bool option_incremental_enabled() const                       = 0;
  virtual bool option_model_gen_enabled() const                         = 0;
  virtual bool option_unsat_assumptions_enabled() const                 = 0;
  virtual std::string get_option_value_enable_incremental() const       = 0;
  virtual std::string get_option_value_enable_model_gen() const         = 0;
  virtual std::string get_option_value_enable_unsat_assumptions() const = 0;

  virtual bool check_failed_assumption(const Term& t) const = 0;

  virtual std::vector<Term> get_value(std::vector<Term>& terms) const = 0;

  //
  // get_model()
  // get_proof()
  // get_unsat_core()
  //
  //
 protected:
  RNGenerator& d_rng;

  std::vector<Base> d_bases = {Base::BIN, Base::DEC, Base::HEX};

  std::vector<SpecialValueBV> d_special_values_bv = {
      SpecialValueBV::ZERO,
      SpecialValueBV::ONE,
      SpecialValueBV::ONES,
      SpecialValueBV::MIN_SIGNED,
      SpecialValueBV::MAX_SIGNED};
};

/**
 * Serialize a solver result to given stream.
 */
std::ostream& operator<<(std::ostream& out, const Solver::Result& r);

/* -------------------------------------------------------------------------- */

}  // namespace smtmbt

#endif
