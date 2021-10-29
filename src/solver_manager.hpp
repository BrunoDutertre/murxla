#ifndef __MURXLA__SOLVER_MANAGER_H
#define __MURXLA__SOLVER_MANAGER_H

#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "solver.hpp"
#include "solver_option.hpp"
#include "sort.hpp"
#include "term_db.hpp"
#include "theory.hpp"
#include "util.hpp"

namespace murxla {

namespace statistics {
struct Statistics;
}

/* -------------------------------------------------------------------------- */

class SolverManager
{
 public:
  using SortSet = std::unordered_set<Sort>;

  /* Statistics. */
  struct Stats
  {
    uint32_t inputs = 0; /* values, constants */
    uint32_t vars   = 0; /* variables */
    uint32_t terms  = 0; /* all terms, including inputs */
    uint32_t sorts  = 0; /* all sorts */
  };

  SolverManager(Solver* solver,
                RNGenerator& rng,
                std::ostream& trace,
                SolverOptions& options,
                bool arith_subtyping,
                bool arith_linear,
                bool trace_seeds,
                bool simple_symbols,
                statistics::Statistics* stats,
                const TheoryIdVector& enabled_theories);
  ~SolverManager() = default;

  statistics::Statistics* d_mbt_stats;

  /**
   * Clear all data.
   *
   * This only clears term, sort
   */
  void clear();

  /**
   * Reset op caches used by pick_op_kind;
   */
  void reset_op_cache();

  /** Get solver. */
  Solver& get_solver();

  /** Set random number generator. */
  void set_rng(RNGenerator& rng);
  /** Get random number generator. */
  RNGenerator& get_rng() const;

  /** Get the list of terms for which tracing with get-sort is pending. */
  std::vector<Term>& get_pending_get_sorts();

  /** Get the trace line for the current seed ("set-seed <seed>"). */
  std::string trace_seed() const;

  /** Get set of enabled theories. */
  const TheoryIdSet& get_enabled_theories() const;

  /** Remove theory from set of enabled theories. */
  void disable_theory(TheoryId theory);

  std::ostream& get_trace();

  /** Return true if given option has already been configured. */
  bool is_option_used(const std::string& opt);
  /** Mark given option as already configured. */
  void mark_option_used(const std::string& opt);

  /** Get the number of created terms. */
  uint64_t get_n_terms() const;
  /** Get the number of created terms of given sort kind. */
  uint64_t get_n_terms(SortKind sort_kind);

  /** Add sort to sort databse. */
  void add_sort(Sort& sort, SortKind sort_kind);
  /** Add value to term database. */
  void add_value(Term& term,
                 Sort& sort,
                 SortKind sort_kind,
                 const AbsTerm::SpecialValueKind& value_kind =
                     AbsTerm::SPECIAL_VALUE_NONE);
  /** Add string value of lenght 1. */
  void add_string_char_value(Term& term);
  /** Add input to term database. */
  void add_input(Term& term, Sort& sort, SortKind sort_kind);
  /** Add var to term database. */
  void add_var(Term& term, Sort& sort, SortKind sort_kind);
  /** Add const to term database. */
  void add_const(Term& term, Sort& sort, SortKind sort_kind);
  /** Add non-input term to term database. */
  void add_term(Term& term,
                SortKind sort_kind,
                const std::vector<Term>& args = {});

  /** Pick arbitrary symbol (simple or piped). */
  std::string pick_symbol();

  /**
   * Pick sort kind of existing (= created) sort.
   * Optionally restrict selection to sort kinds with terms only if
   * 'with_terms' is true.
   */
  SortKind pick_sort_kind(bool with_terms = true);
  /**
   * Pick sort kind of existing (= created) sort out of given set of sort
   * kinds.  Asserts that a sort of any of the given kinds exists.
   * Optionally restrict selection to sort kinds with terms only if
   * 'with_terms' is true.
   */
  SortKind pick_sort_kind(const SortKindSet& sort_kinds,
                          bool with_terms = true);

  /**
   * Pick enabled sort kind (and get its data).
   * Only sort kinds of enabled theories are picked.
   * This function does not guarantee that a sort of this kind alreay exists.
   */
  SortKindData& pick_sort_kind_data();
  /**
   * Pick enabled operator kind (and get its data).
   * Only operator kinds of enabled theories are picked.
   */
  Op::Kind pick_op_kind(bool with_terms = true);

  Op& get_op(const Op::Kind& kind);

  /**
   * Return true if
   *  - with_terms = true : Any terms in any enabled theory have been created
   *                        such that an operator of that theory applies.
   *  - with_terms = false: Any theory is enabled.
   */
  bool has_theory(bool with_terms = true);
  /** Pick any of the enabled theories. */
  TheoryId pick_theory(bool with_terms = true);

  /**
   * Pick a value of given sort.
   * Requires that a value of given sort exists.
   */
  Term pick_value(Sort sort);

  /**
   * Pick string value with lenght 1.
   */
  Term pick_string_char_value();

  /**
   * Pick a term of given sort.
   * Requires that terms of this sort exist.
   */
  Term pick_term(Sort sort);
  /**
   * Pick term of given sort kind and scope level.
   * Requires that terms of this sort kind exist.
   */
  Term pick_term(SortKind sort_kind, size_t level);
  /**
   * Pick term of given sort kind.
   * Requires that terms of this sort kind exist.
   */
  Term pick_term(SortKind sort_kind);
  /**
   * Pick any term.
   * Requires that terms of any sort kind exist.
   */
  Term pick_term();
  /**
   * Pick any term from given level.
   * Requires that terms of any sort kind exist.
   */
  Term pick_term(size_t level);

  /**
   * Pick variable from current scope level.
   */
  Term pick_var();

  /**
   * Remove variable from current scope level.
   */
  void remove_var(Term& var);

  /**
   * Pick Boolean term from current scope level.
   */
  Term pick_quant_body();

  /** Add assumption currently assumed. */
  void add_assumption(Term t);

  /**
   * Pick assumption out of the assumed assumptions list.
   * Requires that d_assumptions is not empty.
   */
  Term pick_assumed_assumption();

  /** Reset solver manager state into start mode. */
  void reset();

  /**
   * Reset solver manager state into assert mode.
   *
   * After this call, calling
   *   - get_model()
   *   - get_unsat_assumptions()
   *   - get_unsat_core() and
   *   - get_proof()
   * is not possible until after the next SAT call.
   */
  void reset_sat();

  /** Return true if term database contains any value of given sort. */
  bool has_value(Sort sort) const;

  /** Return true if we already created string values with lenght 1. */
  bool has_string_char_value() const;

  /** Return true if term database contains any term. */
  bool has_term() const;
  /**
   * Return true if term database contains any term of given sort kind at given
   * level.
   */
  bool has_term(SortKind sort_kind, size_t level) const;
  /** Return true if term database contains any term of given sort kind. */
  bool has_term(SortKind sort_kind) const;
  /**
   * Return true if term database contains any term of one of the given sort
   * kinds.
   */
  bool has_term(const SortKindSet& sort_kinds) const;
  /** Return true if term database contains any term of given sort. */
  bool has_term(Sort sort) const;
  /** Return true if d_assumptions is not empty. */
  bool has_assumed() const;
  /** Return true if term database contains a variable. */
  bool has_var() const;
  /**
   * Return true if term database contains a Boolean term in the current scope
   * level.
   */
  bool has_quant_body() const;

  /**
   * Return the Term in the Term database that wraps the same solver term
   * with the given sort and sort kind.
   * Returns a nullptr if given Term is not in the term database.
   *
   * Note: We need this for Terms returned by the solver that are only wrapped
   *       solver terms without sort information.
   */
  Term find_term(Term term, Sort sort, SortKind sort_kind);

  /**
   * Return the term with the given id.
   * Note: We only use this for untracing.
   */
  Term get_term(uint64_t id) const;

  /**
   * Map an id from a trace to an actual term ID.
   * Note: Only used for untracing.
   */
  void register_term(uint64_t untraced_id, uint64_t term_id);

  /**
   * Map an id from a trace to an actual sort ID.
   * Note: Only used for untracing.
   * Returns false if a sort with the given id does not exist.
   */
  bool register_sort(uint64_t untraced_id, uint64_t sort_id);

  /**
   * Pick sort.
   * It is not guaranteed that there exist terms of the returned sort.
   */
  Sort pick_sort();
  /**
   * Pick sort of given sort kind. Optionally restrict selection to sorts
   * with terms only if 'with_terms' is true.
   */
  Sort pick_sort(SortKind sort_kind, bool with_terms = true);
  /**
   * Pick sort of any of the given sort kinds. Optionally restrict selection to
   * sorts with terms only if 'with_terms' is true.
   */
  Sort pick_sort(const SortKindSet& sort_kinds, bool with_terms = true);
  /**
   * Pick sort, but exclude some of them.
   * It is not guaranteed that there exist terms of the returned sort.
   */
  Sort pick_sort_excluding(const SortKindSet& exclude_sorts,
                           bool with_terms = true);
  /**
   * Pick bit-vector sort with given bit-width.  Optionally restrict
   * selection to sorts with terms only if 'with_terms' is true.
   */
  Sort pick_sort_bv(uint32_t bw, bool with_terms = true);
  /**
   * Pick bit-vector sort with given maximum bit-width.  Optionally restrict
   * selection to sorts with terms only if 'with_terms' is true.
   */
  Sort pick_sort_bv_max(uint32_t bw_max, bool with_terms = true);

  /**
   * Return true if any sort has been created.
   * This does not guarantee that any terms have been created.
   */
  bool has_sort() const;
  /**
   * Return true if a sort of given kind exists.
   * This does not guarantee that any terms of this sort have been created.
   */
  bool has_sort(SortKind sort_kind) const;
  /**
   * Return true if a sort of any of the given kinds exists.
   * This does not guarantee that any terms of these sorts have been created.
   */
  bool has_sort(const SortKindSet& sort_kinds) const;
  /**
   * Return true if given sort already exists.
   * This does not guarantee that any terms of this sort have been created.
   */
  bool has_sort(Sort sort) const;
  /**
   * Return true if sorts other than exclude_sorts have been created.
   * This does not guarantee that any terms have been created.
   */
  bool has_sort_excluding(
      const std::unordered_set<SortKind>& exclude_sorts) const;

  /**
   * Return true if a bit-vector sort with given bit-width exists.
   * Optionally restrict selection to sorts with terms only if 'with_terms' is
   * true.
   */
  bool has_sort_bv(uint32_t bw, bool with_terms = true) const;
  /**
   * Return true if a bit-vector sort up to given maximum bit-width exists.
   * Optionally restrict selection to sorts with terms only if 'with_terms' is
   * true.
   */
  bool has_sort_bv_max(uint32_t bw_max, bool with_terms = true) const;

  /**
   * Return the sort with the given id.
   * Note: We only use this for untracing.
   */
  Sort get_sort(uint64_t id) const;

  /**
   * Set d_n_sorts to id.
   * Note: Only used for untracing.
   */
  void set_n_sorts(uint64_t id);

  /**
   * Lookup sort in d_sorts. If a no matching sort is found the given sort is
   * returned.
   */
  Sort find_sort(Sort sort) const;

  /**
   * Pick an option and an option value.
   *
   * If either are given, enforce option or value. If no option or the given
   * option / value can be set, return empty pair.
   */
  std::pair<std::string, std::string> pick_option(std::string name  = "",
                                                  std::string value = "");

  /** Clear set of assumptions. */
  void clear_assumptions();

  /** Add solver option. */
  void add_option(SolverOption* opt);

  /** Statistics. */
  Stats d_stats;

  /** Config ----------------------------------------------------------------
   *
   * Config members are not cleared or reset on reset() / clear().
   */

  /** True if solver treats Int as a subtype of Real. */
  bool d_arith_subtyping = false;
  /** True to restrict arithmetic operators to linear fragment. */
  bool d_arith_linear = false;

  /**
   * True if every non-return trace call should be preceded by a
   * 'set-seed <seed>' line. We need to provide this option in the solver
   * manager for actions to have access to it.
   */
  bool d_trace_seeds = false;

  /**
   * True if all symbols for terms should be of the form '_sX' rather than
   * a random string.
   */
  bool d_simple_symbols = false;

  /** Solver (config) state -------------------------------------------------
   *
   *  All members below are reset / cleared on reset().
   *  All members that are data structures are cleared on clear().
   */

  /**
   * True if incremental solving is enabled.
   * (SMT-LIB: option :incremental).
   */
  bool d_incremental = false;
  /**
   * True if model generation is enabled.
   * (SMT-LIB: option :produce-models).
   */
  bool d_model_gen = false;
  /**
   * True if producing unsat assumptions is enabled.
   * (SMT-LIB: option :produce-unsat-assumptions).
   */
  bool d_unsat_assumptions = false;
  /**
   * True if producing unsat cores is enabled.
   * (SMT-LIB: option :produce-unsat-cores).
   */
  bool d_unsat_cores = false;

  /** The number of scope levels previously pushed. */
  uint32_t d_n_push_levels = 0;

  /**
   * True if a previous check-sat call is still 'active', i.e., if no formulas
   * have been asserted or assumed since.
   * While true it is save to check failed assumptions and query model values.
   */
  bool d_sat_called = false;

  /** The result of the previous sat call. */
  Solver::Result d_sat_result = Solver::Result::UNKNOWN;

  /** The number of check-sat calls issued. */
  uint32_t d_n_sat_calls = 0;

 private:
  /**
   * Determine and populate set of enabled theories.
   * All theories supported by a solvers are by default enabled and can
   * optionally be disabled (the latter is still TODO).
   */
  void add_enabled_theories(const TheoryIdVector& enabled_theories);

  /**
   * Populate sort kinds database with enabled sort kinds.
   * Sort kinds are enabled based on the set of enabled theories.
   */
  void add_sort_kinds();

#if 0
  template <typename TKind,
            typename TKindData,
            typename TKindMap,
            typename TKindVector>
  TKindData& pick_kind(TKindMap& map,
                       TKindVector* kinds1,
                       TKindVector* kinds2 = nullptr);
#endif

  /** Generalized helper to pick a sort or operator kind. */
  template <typename TKind,
            typename TKindData,
            typename TKindMap>
  TKindData& pick_kind(TKindMap& map);

  /**
   * The activated solver.
   * No calls to the API of the underlying solver are issued from the solver
   * manager, only calls to the API of the murxla::Solver object.
   */
  std::unique_ptr<Solver> d_solver;

  /** The random number generator. */
  RNGenerator& d_rng;

  /** The stream to capture the API trace. */
  std::ostream& d_trace;

  /** Config ----------------------------------------------------------------
   *
   * Config members are not cleared or reset on reset() / clear().
   */

  /** The set of enabled sort kinds. Maps SortKind to SortKindData. */
  SortKindMap d_sort_kinds;
  /** The Op::Kind manager. */
  std::unique_ptr<OpKindManager> d_opmgr;

  /** The set of enabled theories. */
  TheoryIdSet d_enabled_theories;

  /** Map of available solver options */
  SolverOptions& d_solver_options;

  /** Solver state ----------------------------------------------------------
   *
   *  All members below are reset / cleared on reset().
   *  All data members that are are cleared on clear().
   */

  /** The solver options that have already been configured. */
  std::unordered_set<std::string> d_used_solver_options;

  /** Term id counter. */
  uint64_t d_n_terms = 0;
  /** Sort id counter. */
  uint64_t d_n_sorts = 0;

  /** Counter to create simple symbol names when option is enabled. */
  uint32_t d_n_symbols = 0;

  /** Solver state data members -------------------- */

  /** Maintain all created sorts. */
  SortSet d_sorts;

  /** Map sort kind -> sorts. */
  std::unordered_map<SortKind, SortSet> d_sort_kind_to_sorts;

  /** Map SortKind to number of created terms of that SortKind. */
  std::unordered_map<SortKind, uint64_t> d_n_sort_terms;

  /** The set of already assumed formulas. */
  std::unordered_set<Term> d_assumptions;

  /** Term database */
  TermDb d_term_db;

  /** Set of currently created string values with length 1. */
  std::unordered_set<Term> d_string_char_values;

  /** Map untraced ids to corresponding Terms. */
  std::unordered_map<uint64_t, Term> d_untraced_terms;

  /** Map untraced ids to corresponding Terms. */
  std::unordered_map<uint64_t, Sort> d_untraced_sorts;

  /**
   * Cache used by pick_op_kind. Caches operator kinds that are currently
   * safe to pick since the required terms to create an operator already exist.
   */
  std::unordered_map<TheoryId, OpKindSet> d_enabled_op_kinds;

  /**
   * Cache used by pick_op_kind. Caches available operator kinds reported
   * by opmgr, but cannot be constructed yet due to missing terms.
   */
  OpKindMap d_available_op_kinds;
};

/* -------------------------------------------------------------------------- */

}  // namespace murxla
#endif
