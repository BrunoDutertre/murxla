#ifdef MURXLA_USE_BITWUZLA

#include "bzla_solver.hpp"

#include <bitset>
#include <cassert>
#include <cstdlib>

#include "config.hpp"
#include "except.hpp"
#include "theory.hpp"
#include "util.hpp"

namespace murxla {
namespace bzla {

/* -------------------------------------------------------------------------- */
/* BzlaSort                                                                   */
/* -------------------------------------------------------------------------- */

BzlaSort::BzlaSort(Bitwuzla* bzla, BitwuzlaSort* sort)
    : d_solver(bzla), d_sort(sort)
{
}

BzlaSort::~BzlaSort() {}

size_t
BzlaSort::hash() const
{
  return bitwuzla_sort_hash(d_sort);
}

bool
BzlaSort::equals(const Sort& other) const
{
  BzlaSort* bzla_sort = dynamic_cast<BzlaSort*>(other.get());
  if (bzla_sort)
  {
    return d_sort == bzla_sort->d_sort;
  }
  return false;
}

bool
BzlaSort::is_array() const
{
  return bitwuzla_sort_is_array(d_sort);
}

bool
BzlaSort::is_bool() const
{
  BitwuzlaSort* s = bitwuzla_mk_bool_sort(d_solver);
  bool res        = s == d_sort;
  return res && d_kind == SORT_BOOL;
}

bool
BzlaSort::is_bv() const
{
  return bitwuzla_sort_is_bv(d_sort);
}

bool
BzlaSort::is_fp() const
{
  return bitwuzla_sort_is_fp(d_sort);
}

bool
BzlaSort::is_fun() const
{
  return bitwuzla_sort_is_fun(d_sort);
}

bool
BzlaSort::is_int() const
{
  return false;
}

bool
BzlaSort::is_real() const
{
  return false;
}

bool
BzlaSort::is_rm() const
{
  return bitwuzla_sort_is_rm(d_sort);
}

bool
BzlaSort::is_string() const
{
  return false;
}

bool
BzlaSort::is_reglan() const
{
  return false;
}

uint32_t
BzlaSort::get_bv_size() const
{
  assert(is_bv());
  uint32_t res = bitwuzla_sort_bv_get_size(d_sort);
  assert(res);
  return res;
}

uint32_t
BzlaSort::get_fp_exp_size() const
{
  assert(is_fp());
  uint32_t res = bitwuzla_sort_fp_get_exp_size(d_sort);
  assert(res);
  return res;
}

uint32_t
BzlaSort::get_fp_sig_size() const
{
  assert(is_fp());
  uint32_t res = bitwuzla_sort_fp_get_sig_size(d_sort);
  assert(res);
  return res;
}

/* -------------------------------------------------------------------------- */
/* BzlaTerm                                                                   */
/* -------------------------------------------------------------------------- */

BzlaTerm::BzlaTerm(BitwuzlaTerm* term) : d_term(term) {}

BzlaTerm::~BzlaTerm() {}

size_t
BzlaTerm::hash() const
{
  return bitwuzla_term_hash(d_term);
}

bool
BzlaTerm::equals(const Term& other) const
{
  BzlaTerm* bzla_term = dynamic_cast<BzlaTerm*>(other.get());
  return bitwuzla_term_is_equal_sort(d_term, bzla_term->d_term);
}

bool
BzlaTerm::is_array() const
{
  return bitwuzla_term_is_array(d_term);
}

bool
BzlaTerm::is_bool() const
{
  return get_sort()->is_bool();
}

bool
BzlaTerm::is_bv() const
{
  return bitwuzla_term_is_bv(d_term);
}

bool
BzlaTerm::is_fp() const
{
  return bitwuzla_term_is_fp(d_term);
}

bool
BzlaTerm::is_fun() const
{
  return bitwuzla_term_is_fun(d_term);
}

bool
BzlaTerm::is_int() const
{
  return get_sort()->is_int();
}

bool
BzlaTerm::is_real() const
{
  return get_sort()->is_real();
}

bool
BzlaTerm::is_rm() const
{
  return bitwuzla_term_is_rm(d_term);
}

bool
BzlaTerm::is_string() const
{
  return get_sort()->is_string();
}

bool
BzlaTerm::is_reglan() const
{
  return get_sort()->is_reglan();
}

/* -------------------------------------------------------------------------- */
/* BzlaSolver                                                                 */
/* -------------------------------------------------------------------------- */

BzlaSolver::~BzlaSolver()
{
  if (d_solver)
  {
    bitwuzla_delete(d_solver);
    d_solver = nullptr;
  }
}

void
BzlaSolver::new_solver()
{
  assert(d_solver == nullptr);
  d_solver = bitwuzla_new();
  init_op_kinds();
  // TODO: initialize options
}

void
BzlaSolver::delete_solver()
{
  assert(d_solver != nullptr);
  bitwuzla_delete(d_solver);
  d_solver = nullptr;
}

Bitwuzla*
BzlaSolver::get_solver()
{
  return d_solver;
}

bool
BzlaSolver::is_initialized() const
{
  return d_solver != nullptr;
}

TheoryIdVector
BzlaSolver::get_supported_theories() const
{
  // TODO enable when Mathias' bitwuzla quantifiers branch is merged back
  return {THEORY_ARRAY,
          THEORY_BV,
          THEORY_BOOL,
          THEORY_FP,
          /*THEORY_QUANT,*/ THEORY_UF};
}

OpKindSet
BzlaSolver::get_unsupported_op_kinds() const
{
  return {Op::FP_TO_REAL};
}

SortKindSet
BzlaSolver::get_unsupported_var_sort_kinds() const
{
  return {SORT_ARRAY, SORT_FUN, SORT_FP};
}

SortKindSet
BzlaSolver::get_unsupported_array_index_sort_kinds() const
{
  return {SORT_ARRAY, SORT_FUN};
}

SortKindSet
BzlaSolver::get_unsupported_array_element_sort_kinds() const
{
  return {SORT_ARRAY, SORT_FUN};
}

SortKindSet
BzlaSolver::get_unsupported_fun_domain_sort_kinds() const
{
  return {SORT_ARRAY, SORT_FUN};
}

Sort
BzlaSolver::mk_sort(SortKind kind)
{
  MURXLA_CHECK_CONFIG(kind == SORT_BOOL || kind == SORT_RM)
      << "unsupported sort kind '" << kind
      << "' as argument to BzlaSolver::mk_sort, expected '" << SORT_BOOL
      << "' or '" << SORT_RM << "'";

  BitwuzlaSort* bzla_res;

  bzla_res = kind == SORT_BOOL ? bitwuzla_mk_bool_sort(d_solver)
                               : bitwuzla_mk_rm_sort(d_solver);
  assert(bzla_res);
  std::shared_ptr<BzlaSort> res(new BzlaSort(d_solver, bzla_res));
  assert(res);
  return res;
}

Sort
BzlaSolver::mk_sort(SortKind kind, uint32_t size)
{
  MURXLA_CHECK_CONFIG(kind == SORT_BV)
      << "unsupported sort kind '" << kind
      << "' as argument to BzlaSolver::mk_sort, expected '" << SORT_BV << "'";

  BitwuzlaSort* bzla_res = bitwuzla_mk_bv_sort(d_solver, size);
  assert(bzla_res);
  std::shared_ptr<BzlaSort> res(new BzlaSort(d_solver, bzla_res));
  assert(res);
  return res;
}

Sort
BzlaSolver::mk_sort(SortKind kind, uint32_t esize, uint32_t ssize)
{
  MURXLA_CHECK_CONFIG(kind == SORT_FP)
      << "unsupported sort kind '" << kind
      << "' as argument to BzlaSolver::mk_sort, expected '" << SORT_FP << "'";

  BitwuzlaSort* bzla_res = bitwuzla_mk_fp_sort(d_solver, esize, ssize);
  assert(bzla_res);
  std::shared_ptr<BzlaSort> res(new BzlaSort(d_solver, bzla_res));
  assert(res);
  return res;
}

Sort
BzlaSolver::mk_sort(SortKind kind, const std::vector<Sort>& sorts)
{
  BitwuzlaSort* bzla_res;

  switch (kind)
  {
    case SORT_ARRAY:
      bzla_res = bitwuzla_mk_array_sort(
          d_solver, get_bzla_sort(sorts[0]), get_bzla_sort(sorts[1]));
      break;

    case SORT_FUN:
    {
      BitwuzlaSort* codomain = get_bzla_sort(sorts.back());
      std::vector<BitwuzlaSort*> domain;
      for (auto it = sorts.begin(); it < sorts.end() - 1; ++it)
      {
        domain.push_back(get_bzla_sort(*it));
      }
      bzla_res = bitwuzla_mk_fun_sort(
          d_solver, domain.size(), domain.data(), codomain);
      break;
    }

    default:
      MURXLA_CHECK_CONFIG(false)
          << "unsupported sort kind '" << kind
          << "' as argument to BzlaSolver::mk_sort, expected '" << SORT_ARRAY
          << "' or '" << SORT_FUN << "'";
  }
  std::shared_ptr<BzlaSort> res(new BzlaSort(d_solver, bzla_res));
  assert(bzla_res);
  assert(res);
  return res;
}

Term
BzlaSolver::mk_var(Sort sort, const std::string& name)
{
  BitwuzlaTerm* bzla_res;
  std::stringstream ss;
  std::string symbol;
  const char* cname = nullptr;

  /* Make sure that symbol is unique. */
  if (!name.empty())
  {
    ss << "sym" << d_num_symbols << "@" << name;
    ++d_num_symbols;
    symbol = ss.str();
    cname  = symbol.c_str();
  }

  bzla_res = bitwuzla_mk_var(d_solver, get_bzla_sort(sort), cname);
  assert(bzla_res);
  std::shared_ptr<BzlaTerm> res(new BzlaTerm(bzla_res));
  assert(res);
  return res;
}

Term
BzlaSolver::mk_const(Sort sort, const std::string& name)
{
  BitwuzlaTerm* bzla_res;
  std::stringstream ss;
  std::string symbol;
  const char* cname = nullptr;

  /* Make sure that symbol is unique. */
  if (!name.empty())
  {
    ss << "sym" << d_num_symbols << "@" << name;
    ++d_num_symbols;
    symbol = ss.str();
    cname  = symbol.c_str();
  }

  bzla_res = bitwuzla_mk_const(d_solver, get_bzla_sort(sort), cname);
  assert(bzla_res);
  if (d_rng.pick_with_prob(1))
  {
    assert(bitwuzla_term_is_equal_sort(bzla_res, bzla_res));
  }
  std::shared_ptr<BzlaTerm> res(new BzlaTerm(bzla_res));
  assert(res);
  return res;
}

Term
BzlaSolver::mk_value(Sort sort, bool value)
{
  MURXLA_CHECK_CONFIG(sort->is_bool())
      << "unexpected sort of kind '" << sort->get_kind()
      << "' as argument to BzlaSolver::mk_value, expected Boolean sort";

  BitwuzlaTerm* bzla_res =
      value ? bitwuzla_mk_true(d_solver) : bitwuzla_mk_false(d_solver);
  assert(bzla_res);
  if (d_rng.pick_with_prob(10))
  {
    if (value)
    {
      check_is_bv_value(Solver::SPECIAL_VALUE_BV_ONE, bzla_res);
    }
    else
    {
      check_is_bv_value(Solver::SPECIAL_VALUE_BV_ZERO, bzla_res);
    }
  }
  std::shared_ptr<BzlaTerm> res(new BzlaTerm(bzla_res));
  assert(res);
  return res;
}

BitwuzlaTerm*
BzlaSolver::mk_value_bv_uint64(Sort sort, uint64_t value)
{
  MURXLA_CHECK_CONFIG(sort->is_bv())
      << "unexpected sort of kind '" << sort->get_kind()
      << "' as argument to BzlaSolver::mk_value, expected bit-vector sort";

  BitwuzlaSort* bzla_sort = get_bzla_sort(sort);
  BitwuzlaTerm* bzla_res =
      bitwuzla_mk_bv_value_uint64(d_solver, bzla_sort, value);
  assert(bzla_res);
  return bzla_res;
}

Term
BzlaSolver::mk_value(Sort sort, std::string value, Base base)
{
  MURXLA_CHECK_CONFIG(sort->is_bv())
      << "unexpected sort of kind '" << sort->get_kind()
      << "' as argument to BzlaSolver::mk_value, expected bit-vector sort";

  BitwuzlaTerm* bzla_res;
  BitwuzlaSort* bzla_sort = get_bzla_sort(sort);
  uint32_t bw             = sort->get_bv_size();
  int32_t ibase;
  BitwuzlaBVBase cbase;

  if (base == DEC)
  {
    ibase = 10;
    cbase = BITWUZLA_BV_BASE_DEC;
  }
  else if (base == HEX)
  {
    ibase = 16;
    cbase = BITWUZLA_BV_BASE_HEX;
  }
  else
  {
    assert(base == BIN);
    ibase = 2;
    cbase = BITWUZLA_BV_BASE_BIN;
  }

  if (bw <= 64 && d_rng.flip_coin())
  {
    bzla_res =
        mk_value_bv_uint64(sort, strtoull(value.c_str(), nullptr, ibase));
  }
  else
  {
    bzla_res = bitwuzla_mk_bv_value(d_solver, bzla_sort, value.c_str(), cbase);
  }
  assert(bzla_res);
  std::shared_ptr<BzlaTerm> res(new BzlaTerm(bzla_res));
  assert(res);
  return res;
}

Term
BzlaSolver::mk_special_value(Sort sort, const SpecialValueKind& value)
{
  BitwuzlaTerm* bzla_res  = 0;
  BitwuzlaSort* bzla_sort = get_bzla_sort(sort);
  bool check              = d_rng.pick_with_prob(10);
  std::string str;

  switch (sort->get_kind())
  {
    case SORT_BV:
      if (value == SPECIAL_VALUE_BV_ZERO)
      {
        bzla_res = bitwuzla_mk_bv_zero(d_solver, bzla_sort);
        if (check) check_is_bv_value(Solver::SPECIAL_VALUE_BV_ZERO, bzla_res);
      }
      else if (value == SPECIAL_VALUE_BV_ONE)
      {
        bzla_res = bitwuzla_mk_bv_one(d_solver, bzla_sort);
        if (check) check_is_bv_value(Solver::SPECIAL_VALUE_BV_ONE, bzla_res);
      }
      else if (value == SPECIAL_VALUE_BV_ONES)
      {
        bzla_res = bitwuzla_mk_bv_ones(d_solver, bzla_sort);
        if (check) check_is_bv_value(Solver::SPECIAL_VALUE_BV_ONES, bzla_res);
      }
      else if (value == SPECIAL_VALUE_BV_MIN_SIGNED)
      {
        bzla_res = bitwuzla_mk_bv_min_signed(d_solver, bzla_sort);
        if (check)
          check_is_bv_value(Solver::SPECIAL_VALUE_BV_MIN_SIGNED, bzla_res);
      }
      else
      {
        assert(value == SPECIAL_VALUE_BV_MAX_SIGNED);
        bzla_res = bitwuzla_mk_bv_max_signed(d_solver, bzla_sort);
        if (check)
          check_is_bv_value(Solver::SPECIAL_VALUE_BV_MAX_SIGNED, bzla_res);
      }
      break;

    case SORT_FP:
    {
      if (value == SPECIAL_VALUE_FP_POS_INF)
      {
        bzla_res = bitwuzla_mk_fp_pos_inf(d_solver, bzla_sort);
      }
      else if (value == SPECIAL_VALUE_FP_NEG_INF)
      {
        bzla_res = bitwuzla_mk_fp_neg_inf(d_solver, bzla_sort);
      }
      else if (value == SPECIAL_VALUE_FP_POS_ZERO)
      {
        bzla_res = bitwuzla_mk_fp_pos_zero(d_solver, bzla_sort);
      }
      else if (value == SPECIAL_VALUE_FP_NEG_ZERO)
      {
        bzla_res = bitwuzla_mk_fp_neg_zero(d_solver, bzla_sort);
      }
      else
      {
        assert(value == SPECIAL_VALUE_FP_NAN);
        bzla_res = bitwuzla_mk_fp_nan(d_solver, bzla_sort);
      }
    }
    break;

    case SORT_RM:
      if (value == SPECIAL_VALUE_RM_RNA)
      {
        bzla_res = bitwuzla_mk_rm_value(d_solver, BITWUZLA_RM_RNA);
      }
      else if (value == SPECIAL_VALUE_RM_RNE)
      {
        bzla_res = bitwuzla_mk_rm_value(d_solver, BITWUZLA_RM_RNE);
      }
      else if (value == SPECIAL_VALUE_RM_RTN)
      {
        bzla_res = bitwuzla_mk_rm_value(d_solver, BITWUZLA_RM_RTN);
      }
      else if (value == SPECIAL_VALUE_RM_RTP)
      {
        bzla_res = bitwuzla_mk_rm_value(d_solver, BITWUZLA_RM_RTP);
      }
      else
      {
        assert(value == SPECIAL_VALUE_RM_RTZ);
        bzla_res = bitwuzla_mk_rm_value(d_solver, BITWUZLA_RM_RTZ);
      }
      break;

    default:
      MURXLA_CHECK_CONFIG(sort->is_bv())
          << "unexpected sort of kind '" << sort->get_kind()
          << "' as argument to BzlaSolver::mk_special_value, expected "
             "bit-vector, floating-point or RoundingMode sort";
  }

  assert(bzla_res);
  std::shared_ptr<BzlaTerm> res(new BzlaTerm(bzla_res));
  assert(res);
  return res;
}

Term
BzlaSolver::mk_term(const OpKind& kind,
                    std::vector<Term>& args,
                    std::vector<uint32_t>& params)
{
  MURXLA_CHECK_CONFIG(d_op_kinds.find(kind) != d_op_kinds.end())
      << "BzlaSolver: operator kind '" << kind << "' not configured";

  BitwuzlaTerm* bzla_res = nullptr;
  size_t n_args          = args.size();
  size_t n_params        = params.size();
  BitwuzlaKind bzla_kind = d_op_kinds.at(kind);
  std::vector<BitwuzlaTerm*> vars;
  std::vector<BitwuzlaTerm*> bzla_args = terms_to_bzla_terms(args);

  if (n_params)
  {
    bzla_res = bitwuzla_mk_term_indexed(
        d_solver, bzla_kind, n_args, bzla_args.data(), n_params, params.data());
  }
  else
  {
    bzla_res = bitwuzla_mk_term(d_solver, bzla_kind, n_args, bzla_args.data());
  }
  assert(bzla_res);
  std::shared_ptr<BzlaTerm> res(new BzlaTerm(bzla_res));
  assert(res);
  return res;
}

Sort
BzlaSolver::get_sort(Term term, SortKind sort_kind) const
{
  (void) sort_kind;
  return std::shared_ptr<BzlaSort>(
      new BzlaSort(d_solver, bitwuzla_term_get_sort(get_bzla_term(term))));
}

void
BzlaSolver::assert_formula(const Term& t)
{
  bitwuzla_assert(d_solver, get_bzla_term(t));
}

Solver::Result
BzlaSolver::check_sat()
{
  BitwuzlaResult res = bitwuzla_check_sat(d_solver);
  if (res == BITWUZLA_SAT) return Result::SAT;
  if (res == BITWUZLA_UNSAT) return Result::UNSAT;
  assert(res == BITWUZLA_UNKNOWN);
  return Result::UNKNOWN;
}

Solver::Result
BzlaSolver::check_sat_assuming(std::vector<Term>& assumptions)
{
  int32_t res;
  for (const Term& t : assumptions)
  {
    bitwuzla_assume(d_solver, get_bzla_term(t));
  }
  res = bitwuzla_check_sat(d_solver);
  if (res == BITWUZLA_SAT) return Result::SAT;
  if (res == BITWUZLA_UNSAT) return Result::UNSAT;
  assert(res == BITWUZLA_UNKNOWN);
  return Result::UNKNOWN;
}

std::vector<Term>
BzlaSolver::get_unsat_assumptions()
{
  size_t n_assumptions;
  std::vector<Term> res;
  BitwuzlaTerm** bzla_res =
      bitwuzla_get_unsat_assumptions(d_solver, &n_assumptions);
  for (uint32_t i = 0; i < n_assumptions; ++i)
  {
    res.push_back(
        std::shared_ptr<BzlaTerm>(new BzlaTerm((BitwuzlaTerm*) bzla_res[i])));
  }
  return res;
}

std::vector<Term>
BzlaSolver::get_value(std::vector<Term>& terms)
{
  std::vector<Term> res;
  std::vector<BitwuzlaTerm*> bzla_res;
  std::vector<BitwuzlaTerm*> bzla_terms = terms_to_bzla_terms(terms);

  for (BitwuzlaTerm* t : bzla_terms)
  {
    bzla_res.push_back(bitwuzla_get_value(d_solver, t));
  }
  return bzla_terms_to_terms(bzla_res);
}

void
BzlaSolver::push(uint32_t n_levels)
{
  bitwuzla_push(d_solver, n_levels);
}

void
BzlaSolver::pop(uint32_t n_levels)
{
  bitwuzla_pop(d_solver, n_levels);
}

void
BzlaSolver::print_model()
{
  const char* fmt = d_rng.flip_coin() ? "btor" : "smt2";
  bitwuzla_print_model(d_solver, (char*) fmt, stdout);
}

void
BzlaSolver::reset_assertions()
{
  /* Bitwuzla does not support this yet */
}

/* -------------------------------------------------------------------------- */

bool
BzlaSolver::check_unsat_assumption(const Term& t) const
{
  return bitwuzla_is_unsat_assumption(d_solver, get_bzla_term(t));
}

/* -------------------------------------------------------------------------- */

BitwuzlaSort*
BzlaSolver::get_bzla_sort(Sort sort) const
{
  BzlaSort* bzla_sort = dynamic_cast<BzlaSort*>(sort.get());
  assert(bzla_sort);
  return bzla_sort->d_sort;
}

BitwuzlaTerm*
BzlaSolver::get_bzla_term(Term term) const
{
  BzlaTerm* bzla_term = dynamic_cast<BzlaTerm*>(term.get());
  assert(bzla_term);
  return bzla_term->d_term;
}

void
BzlaSolver::set_opt(const std::string& opt, const std::string& value)
{
  if (opt == "produce-unsat-assumptions")
  {
    /* always enabled in Bitwuzla, can not be configured via set_opt */
    return;
  }

  // TODO reenable after option fuzzing for bitwuzla is configured
  // assert(d_option_name_to_enum.find(opt) != d_option_name_to_enum.end());

  /* Bitwuzla options are all integer values */
  uint32_t val = 0;
  BitwuzlaOption bzla_opt;

  val = value == "true" ? 1 : std::stoul(value);
  // TODO support all options
  if (opt == "produce-models")
  {
    bzla_opt = BITWUZLA_OPT_PRODUCE_MODELS;
  }
  else if (opt == "incremental")
  {
    bzla_opt = BITWUZLA_OPT_INCREMENTAL;
  }
  else
  {
    return;
  }
  bitwuzla_set_option(d_solver, bzla_opt, val);
  assert(val == bitwuzla_get_option(d_solver, bzla_opt));
}

std::string
BzlaSolver::get_option_name_incremental() const
{
  return "incremental";
}

std::string
BzlaSolver::get_option_name_model_gen() const
{
  return "produce-models";
}

std::string
BzlaSolver::get_option_name_unsat_assumptions() const
{
  /* always enabled in Bitwuzla, can not be configured via set_opt */
  return "produce-unsat-assumptions";
}

bool
BzlaSolver::option_incremental_enabled() const
{
  return bitwuzla_get_option(d_solver, BITWUZLA_OPT_INCREMENTAL) > 0;
}

bool
BzlaSolver::option_model_gen_enabled() const
{
  return bitwuzla_get_option(d_solver, BITWUZLA_OPT_PRODUCE_MODELS) > 0;
}

bool
BzlaSolver::option_unsat_assumptions_enabled() const
{
  /* always enabled in Bitwuzla, can not be configured via set_opt */
  return true;
}

/* -------------------------------------------------------------------------- */

void
BzlaSolver::init_op_kinds()
{
  d_op_kinds = {
      /* Special Cases */
      {Op::DISTINCT, BITWUZLA_KIND_DISTINCT},
      {Op::EQUAL, BITWUZLA_KIND_EQUAL},
      {Op::ITE, BITWUZLA_KIND_ITE},

      /* Bool */
      {Op::AND, BITWUZLA_KIND_AND},
      {Op::OR, BITWUZLA_KIND_OR},
      {Op::NOT, BITWUZLA_KIND_NOT},
      {Op::XOR, BITWUZLA_KIND_XOR},
      {Op::IMPLIES, BITWUZLA_KIND_IMPLIES},

      /* Arrays */
      {Op::ARRAY_SELECT, BITWUZLA_KIND_ARRAY_SELECT},
      {Op::ARRAY_STORE, BITWUZLA_KIND_ARRAY_STORE},

      /* BV */
      {Op::BV_EXTRACT, BITWUZLA_KIND_BV_EXTRACT},
      {Op::BV_REPEAT, BITWUZLA_KIND_BV_REPEAT},
      {Op::BV_ROTATE_LEFT, BITWUZLA_KIND_BV_ROLI},
      {Op::BV_ROTATE_RIGHT, BITWUZLA_KIND_BV_RORI},
      {Op::BV_SIGN_EXTEND, BITWUZLA_KIND_BV_SIGN_EXTEND},
      {Op::BV_ZERO_EXTEND, BITWUZLA_KIND_BV_ZERO_EXTEND},

      {Op::BV_CONCAT, BITWUZLA_KIND_BV_CONCAT},
      {Op::BV_AND, BITWUZLA_KIND_BV_AND},
      {Op::BV_OR, BITWUZLA_KIND_BV_OR},
      {Op::BV_XOR, BITWUZLA_KIND_BV_XOR},
      {Op::BV_MULT, BITWUZLA_KIND_BV_MUL},
      {Op::BV_ADD, BITWUZLA_KIND_BV_ADD},
      {Op::BV_NOT, BITWUZLA_KIND_BV_NOT},
      {Op::BV_NEG, BITWUZLA_KIND_BV_NEG},
      {Op::BV_NAND, BITWUZLA_KIND_BV_NAND},
      {Op::BV_NOR, BITWUZLA_KIND_BV_NOR},
      {Op::BV_XNOR, BITWUZLA_KIND_BV_XNOR},
      {Op::BV_COMP, BITWUZLA_KIND_BV_COMP},
      {Op::BV_SUB, BITWUZLA_KIND_BV_SUB},
      {Op::BV_UDIV, BITWUZLA_KIND_BV_UDIV},
      {Op::BV_UREM, BITWUZLA_KIND_BV_UREM},
      {Op::BV_UREM, BITWUZLA_KIND_BV_UREM},
      {Op::BV_SDIV, BITWUZLA_KIND_BV_SDIV},
      {Op::BV_SREM, BITWUZLA_KIND_BV_SREM},
      {Op::BV_SMOD, BITWUZLA_KIND_BV_SMOD},
      {Op::BV_SHL, BITWUZLA_KIND_BV_SHL},
      {Op::BV_LSHR, BITWUZLA_KIND_BV_SHR},
      {Op::BV_ASHR, BITWUZLA_KIND_BV_ASHR},
      {Op::BV_ULT, BITWUZLA_KIND_BV_ULT},
      {Op::BV_ULE, BITWUZLA_KIND_BV_ULE},
      {Op::BV_UGT, BITWUZLA_KIND_BV_UGT},
      {Op::BV_UGE, BITWUZLA_KIND_BV_UGE},
      {Op::BV_SLT, BITWUZLA_KIND_BV_SLT},
      {Op::BV_SLE, BITWUZLA_KIND_BV_SLE},
      {Op::BV_SGT, BITWUZLA_KIND_BV_SGT},
      {Op::BV_SGE, BITWUZLA_KIND_BV_SGE},

      /* FP */
      {Op::FP_ABS, BITWUZLA_KIND_FP_ABS},
      {Op::FP_ADD, BITWUZLA_KIND_FP_ADD},
      {Op::FP_DIV, BITWUZLA_KIND_FP_DIV},
      {Op::FP_EQ, BITWUZLA_KIND_FP_EQ},
      {Op::FP_FMA, BITWUZLA_KIND_FP_FMA},
      {Op::FP_FP, BITWUZLA_KIND_FP_FP},
      {Op::FP_IS_NORMAL, BITWUZLA_KIND_FP_IS_NORMAL},
      {Op::FP_IS_SUBNORMAL, BITWUZLA_KIND_FP_IS_SUBNORMAL},
      {Op::FP_IS_INF, BITWUZLA_KIND_FP_IS_INF},
      {Op::FP_IS_NAN, BITWUZLA_KIND_FP_IS_NAN},
      {Op::FP_IS_NEG, BITWUZLA_KIND_FP_IS_NEG},
      {Op::FP_IS_POS, BITWUZLA_KIND_FP_IS_POS},
      {Op::FP_IS_ZERO, BITWUZLA_KIND_FP_IS_ZERO},
      {Op::FP_LT, BITWUZLA_KIND_FP_LT},
      {Op::FP_LEQ, BITWUZLA_KIND_FP_LEQ},
      {Op::FP_GT, BITWUZLA_KIND_FP_GT},
      {Op::FP_GEQ, BITWUZLA_KIND_FP_GEQ},
      {Op::FP_MAX, BITWUZLA_KIND_FP_MAX},
      {Op::FP_MIN, BITWUZLA_KIND_FP_MIN},
      {Op::FP_MUL, BITWUZLA_KIND_FP_MUL},
      {Op::FP_NEG, BITWUZLA_KIND_FP_NEG},
      {Op::FP_REM, BITWUZLA_KIND_FP_REM},
      {Op::FP_RTI, BITWUZLA_KIND_FP_RTI},
      {Op::FP_SQRT, BITWUZLA_KIND_FP_SQRT},
      {Op::FP_SUB, BITWUZLA_KIND_FP_SUB},
      {Op::FP_TO_FP_FROM_BV, BITWUZLA_KIND_FP_TO_FP_FROM_BV},
      {Op::FP_TO_FP_FROM_SBV, BITWUZLA_KIND_FP_TO_FP_FROM_SBV},
      {Op::FP_TO_FP_FROM_FP, BITWUZLA_KIND_FP_TO_FP_FROM_FP},
      {Op::FP_TO_FP_FROM_UBV, BITWUZLA_KIND_FP_TO_FP_FROM_UBV},
      {Op::FP_TO_SBV, BITWUZLA_KIND_FP_TO_SBV},
      {Op::FP_TO_UBV, BITWUZLA_KIND_FP_TO_UBV},

      /* Quantifiers */
      {Op::FORALL, BITWUZLA_KIND_FORALL},
      {Op::EXISTS, BITWUZLA_KIND_EXISTS},

      /* UF */
      {Op::UF_APPLY, BITWUZLA_KIND_APPLY},

      /* Solver-specific operators */
      {OP_BV_DEC, BITWUZLA_KIND_BV_DEC},
      {OP_BV_INC, BITWUZLA_KIND_BV_INC},
      {OP_BV_ROL, BITWUZLA_KIND_BV_ROL},
      {OP_BV_ROR, BITWUZLA_KIND_BV_ROR},
      {OP_BV_REDAND, BITWUZLA_KIND_BV_REDAND},
      {OP_BV_REDOR, BITWUZLA_KIND_BV_REDOR},
      {OP_BV_REDXOR, BITWUZLA_KIND_BV_REDXOR},
      {OP_BV_UADDO, BITWUZLA_KIND_BV_UADD_OVERFLOW},
      {OP_BV_SADDO, BITWUZLA_KIND_BV_SADD_OVERFLOW},
      {OP_BV_UMULO, BITWUZLA_KIND_BV_UMUL_OVERFLOW},
      {OP_BV_SMULO, BITWUZLA_KIND_BV_SMUL_OVERFLOW},
      {OP_BV_USUBO, BITWUZLA_KIND_BV_USUB_OVERFLOW},
      {OP_BV_SSUBO, BITWUZLA_KIND_BV_SSUB_OVERFLOW},
      {OP_BV_SDIVO, BITWUZLA_KIND_BV_SDIV_OVERFLOW},
  };
}

std::vector<Term>
BzlaSolver::bzla_terms_to_terms(std::vector<BitwuzlaTerm*>& terms) const
{
  std::vector<Term> res;
  for (BitwuzlaTerm* t : terms)
  {
    res.push_back(std::shared_ptr<BzlaTerm>(new BzlaTerm(t)));
  }
  return res;
}

std::vector<BitwuzlaTerm*>
BzlaSolver::terms_to_bzla_terms(std::vector<Term>& terms) const
{
  std::vector<BitwuzlaTerm*> res;
  for (Term& t : terms)
  {
    res.push_back(get_bzla_term(t));
  }
  return res;
}

BzlaSolver::BzlaTermFunBoolUnary
BzlaSolver::pick_fun_bool_unary(BzlaTermFunBoolUnaryVector& funs) const
{
  return d_rng.pick_from_set<BzlaTermFunBoolUnaryVector, BzlaTermFunBoolUnary>(
      funs);
}

BzlaSolver::BzlaTermFunBoolUnary
BzlaSolver::pick_fun_is_bv_const() const
{
  BzlaTermFunBoolUnaryVector funs = {bitwuzla_term_is_bv_value_zero,
                                     bitwuzla_term_is_bv_value_one,
                                     bitwuzla_term_is_bv_value_ones,
                                     bitwuzla_term_is_bv_value_max_signed,
                                     bitwuzla_term_is_bv_value_min_signed};
  return pick_fun_bool_unary(funs);
}

void
BzlaSolver::check_is_bv_value(const Solver::SpecialValueKind& kind,
                              BitwuzlaTerm* node) const
{
  uint32_t bw              = bitwuzla_term_bv_get_size(node);
  RNGenerator::Choice pick = d_rng.pick_one_of_three();

  if (pick == RNGenerator::Choice::FIRST)
  {
    BzlaTermFunBoolUnaryVector is_funs;
    BzlaTermFunBoolUnaryVector is_not_funs;
    if (kind == Solver::SPECIAL_VALUE_BV_ONE)
    {
      is_funs.push_back(bitwuzla_term_is_bv_value_one);
      if (bw > 1)
      {
        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_min_signed);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
      else
      {
        is_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_funs.push_back(bitwuzla_term_is_bv_value_min_signed);

        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
    }
    else if (kind == Solver::SPECIAL_VALUE_BV_ONES)
    {
      is_funs.push_back(bitwuzla_term_is_bv_value_ones);
      if (bw > 1)
      {
        is_not_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_min_signed);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
      else
      {
        is_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_funs.push_back(bitwuzla_term_is_bv_value_min_signed);

        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
    }
    else if (kind == Solver::SPECIAL_VALUE_BV_ZERO)
    {
      is_funs.push_back(bitwuzla_term_is_bv_value_zero);
      if (bw > 1)
      {
        is_not_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_min_signed);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
      else
      {
        is_funs.push_back(bitwuzla_term_is_bv_value_max_signed);

        is_not_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_min_signed);
      }
    }
    else if (kind == Solver::SPECIAL_VALUE_BV_MIN_SIGNED)
    {
      is_funs.push_back(bitwuzla_term_is_bv_value_min_signed);
      if (bw > 1)
      {
        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
      else
      {
        is_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_funs.push_back(bitwuzla_term_is_bv_value_ones);

        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
    }
    else
    {
      assert(kind == Solver::SPECIAL_VALUE_BV_MAX_SIGNED);
      is_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      if (bw > 1)
      {
        is_not_funs.push_back(bitwuzla_term_is_bv_value_zero);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_min_signed);
      }
      else
      {
        is_funs.push_back(bitwuzla_term_is_bv_value_zero);

        is_not_funs.push_back(bitwuzla_term_is_bv_value_one);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_ones);
        is_not_funs.push_back(bitwuzla_term_is_bv_value_max_signed);
      }
    }
    if (d_rng.flip_coin())
    {
      assert(pick_fun_bool_unary(is_funs)(node));
    }
    else
    {
      assert(!pick_fun_bool_unary(is_not_funs)(node));
    }
  }
  else if (pick == RNGenerator::Choice::SECOND)
  {
    assert(bitwuzla_term_is_bv_value(node));
  }
  else
  {
    assert(pick == RNGenerator::Choice::THIRD);
    assert(!bitwuzla_term_is_const(node));
  }
}

/* -------------------------------------------------------------------------- */
/* Solver-specific operators, SolverManager configuration.                    */
/* -------------------------------------------------------------------------- */

const OpKind BzlaSolver::OP_BV_DEC    = "bzla-OP_BV_DEC";
const OpKind BzlaSolver::OP_BV_INC    = "bzla-OP_BV_INC";
const OpKind BzlaSolver::OP_BV_REDAND = "bzla-OP_BV_REDAND";
const OpKind BzlaSolver::OP_BV_REDOR  = "bzla-OP_BV_REDOR";
const OpKind BzlaSolver::OP_BV_REDXOR = "bzla-OP_BV_REDXOR";
const OpKind BzlaSolver::OP_BV_ROL    = "bzla-OP_BV_ROL";
const OpKind BzlaSolver::OP_BV_ROR    = "bzla-OP_BV_ROR";
const OpKind BzlaSolver::OP_BV_SADDO  = "bzla-OP_BV_SADDO";
const OpKind BzlaSolver::OP_BV_SDIVO  = "bzla-OP_BV_SDIVO";
const OpKind BzlaSolver::OP_BV_SMULO  = "bzla-OP_BV_SMULO";
const OpKind BzlaSolver::OP_BV_SSUBO  = "bzla-OP_BV_SSUBO";
const OpKind BzlaSolver::OP_BV_UADDO  = "bzla-OP_BV_UADDO";
const OpKind BzlaSolver::OP_BV_UMULO  = "bzla-OP_BV_UMULO";
const OpKind BzlaSolver::OP_BV_USUBO  = "bzla-OP_BV_USUBO";

void
BzlaSolver::configure_smgr(SolverManager* smgr) const
{
  smgr->add_op_kind(OP_BV_DEC, 1, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_INC, 1, 0, SORT_BV, {SORT_BV}, THEORY_BV);

  smgr->add_op_kind(OP_BV_REDAND, 1, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_REDOR, 1, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_REDXOR, 1, 0, SORT_BV, {SORT_BV}, THEORY_BV);

  smgr->add_op_kind(OP_BV_UADDO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_UMULO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_USUBO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_SADDO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_SDIVO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_SMULO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
  smgr->add_op_kind(OP_BV_SSUBO, 2, 0, SORT_BV, {SORT_BV}, THEORY_BV);
}

/* -------------------------------------------------------------------------- */
/* Solver-specific actions and states, FSM configuration.                     */
/* -------------------------------------------------------------------------- */

/* solver-specific actions */
const ActionKind BzlaSolver::ACTION_IS_UNSAT_ASSUMPTION =
    "bzla-is-unsat-assumption";
const ActionKind BzlaSolver::ACTION_FIXATE_ASSUMPTIONS =
    "bzla-fixate-assumptions";
const ActionKind BzlaSolver::ACTION_RESET_ASSUMPTIONS =
    "bzla-reset-assumptions";
const ActionKind BzlaSolver::ACTION_SIMPLIFY        = "bzla-simplify";
const ActionKind BzlaSolver::ACTION_TERM_SET_SYMBOL = "bzla-term-set-symbol";
/* solver-specific states */
const StateKind BzlaSolver::STATE_FIX_RESET_ASSUMPTIONS =
    "bzla-fix-reset-assumptions";

class BzlaActionIsUnsatAssumption : public Action
{
 public:
  BzlaActionIsUnsatAssumption(SolverManager& smgr)
      : Action(smgr, BzlaSolver::ACTION_IS_UNSAT_ASSUMPTION, false)
  {
  }

  bool run() override
  {
    assert(d_solver.is_initialized());
    if (!d_smgr.d_sat_called) return false;
    if (d_smgr.d_sat_result != Solver::Result::UNSAT) return false;
    if (!d_smgr.d_incremental) return false;
    if (!d_smgr.has_assumed()) return false;
    Term term = d_smgr.pick_assumed_assumption();
    _run(term);
    return true;
  }

  uint64_t untrace(std::vector<std::string>& tokens) override
  {
    MURXLA_CHECK_TRACE_NTOKENS(1, tokens.size());
    Term term = d_smgr.get_term(str_to_uint32(tokens[0]));
    MURXLA_CHECK_TRACE_TERM(term, tokens[0]);
    _run(term);
    return 0;
  }

 private:
  void _run(Term term)
  {
    MURXLA_TRACE << get_kind() << " " << term;
    BzlaSolver& bzla_solver = static_cast<BzlaSolver&>(d_smgr.get_solver());
    (void) bitwuzla_is_unsat_assumption(bzla_solver.get_solver(),
                                        bzla_solver.get_bzla_term(term));
  }
};

class BzlaActionFixateAssumptions : public Action
{
 public:
  BzlaActionFixateAssumptions(SolverManager& smgr)
      : Action(smgr, BzlaSolver::ACTION_FIXATE_ASSUMPTIONS, false)
  {
  }

  bool run() override
  {
    assert(d_solver.is_initialized());
    if (!d_smgr.d_incremental) return false;
    _run();
    return true;
  }

  uint64_t untrace(std::vector<std::string>& tokens) override
  {
    MURXLA_CHECK_TRACE_EMPTY(tokens);
    _run();
    return 0;
  }

 private:
  void _run()
  {
    MURXLA_TRACE << get_kind();
    d_smgr.clear();
    bitwuzla_fixate_assumptions(
        static_cast<BzlaSolver&>(d_smgr.get_solver()).get_solver());
  }
};

class BzlaActionResetAssumptions : public Action
{
 public:
  BzlaActionResetAssumptions(SolverManager& smgr)
      : Action(smgr, BzlaSolver::ACTION_RESET_ASSUMPTIONS, false)
  {
  }

  bool run() override
  {
    assert(d_solver.is_initialized());
    if (!d_smgr.d_incremental) return false;
    _run();
    return true;
  }

  uint64_t untrace(std::vector<std::string>& tokens) override
  {
    MURXLA_CHECK_TRACE_EMPTY(tokens);
    _run();
    return 0;
  }

 private:
  void _run()
  {
    MURXLA_TRACE << get_kind();
    d_smgr.clear();
    bitwuzla_reset_assumptions(
        static_cast<BzlaSolver&>(d_smgr.get_solver()).get_solver());
  }
};

class BzlaActionSimplify : public Action
{
 public:
  BzlaActionSimplify(SolverManager& smgr)
      : Action(smgr, BzlaSolver::ACTION_SIMPLIFY, false)
  {
  }

  bool run() override
  {
    assert(d_solver.is_initialized());
    BzlaSolver& solver = static_cast<BzlaSolver&>(d_smgr.get_solver());
    if (solver.get_solver() == nullptr) return false;
    _run();
    return true;
  }

  uint64_t untrace(std::vector<std::string>& tokens) override
  {
    MURXLA_CHECK_TRACE_EMPTY(tokens);
    _run();
    return 0;
  }

 private:
  void _run()
  {
    MURXLA_TRACE << get_kind();
    bitwuzla_simplify(
        static_cast<BzlaSolver&>(d_smgr.get_solver()).get_solver());
  }
};

class BzlaActionTermSetSymbol : public Action
{
 public:
  BzlaActionTermSetSymbol(SolverManager& smgr)
      : Action(smgr, BzlaSolver::ACTION_TERM_SET_SYMBOL, false)
  {
  }

  bool run() override
  {
    assert(d_solver.is_initialized());
    if (!d_smgr.has_term()) return false;
    Term term          = d_smgr.pick_term();
    std::string symbol = d_smgr.pick_symbol();
    _run(term, symbol);
    return true;
  }

  uint64_t untrace(std::vector<std::string>& tokens) override
  {
    MURXLA_CHECK_TRACE_NTOKENS(2, tokens.size());
    Term term = d_smgr.get_term(str_to_uint32(tokens[0]));
    MURXLA_CHECK_TRACE_TERM(term, tokens[0]);
    std::string symbol = str_to_str(tokens[1]);
    _run(term, symbol);
    return 0;
  }

 private:
  void _run(Term term, std::string symbol)
  {
    MURXLA_TRACE << get_kind() << " " << term << " \"" << symbol << "\"";
    BzlaSolver& bzla_solver = static_cast<BzlaSolver&>(d_smgr.get_solver());
    (void) bitwuzla_term_set_symbol(bzla_solver.get_bzla_term(term),
                                    symbol.c_str());
  }
};

/* -------------------------------------------------------------------------- */

void
BzlaSolver::configure_fsm(FSM* fsm) const
{
  State* s_assert                = fsm->get_state(State::ASSERT);
  State* s_fix_reset_assumptions = fsm->new_state(STATE_FIX_RESET_ASSUMPTIONS);

  auto t_default = fsm->new_action<TransitionDefault>();

  // bitwuzla_is_unsat_assumption
  auto a_failed = fsm->new_action<BzlaActionIsUnsatAssumption>();
  fsm->add_action_to_all_states(a_failed, 100);

  // bitwuzla_fixate_assumptions
  // bitwuzla_reset_assumptions
  auto a_fix_assumptions   = fsm->new_action<BzlaActionFixateAssumptions>();
  auto a_reset_assumptions = fsm->new_action<BzlaActionResetAssumptions>();
  s_fix_reset_assumptions->add_action(a_fix_assumptions, 5);
  s_fix_reset_assumptions->add_action(a_reset_assumptions, 5);
  s_fix_reset_assumptions->add_action(t_default, 1, s_assert);
  fsm->add_action_to_all_states_next(
      t_default, 2, s_fix_reset_assumptions, {State::OPT});

  // bitwuzla_simplify
  auto a_simplify = fsm->new_action<BzlaActionSimplify>();
  fsm->add_action_to_all_states(a_simplify, 100);

  // bitwuzla_term_set_symbol
  auto a_set_symbol = fsm->new_action<BzlaActionTermSetSymbol>();
  fsm->add_action_to_all_states(a_set_symbol, 100);
}

}  // namespace bzla
}  // namespace murxla

#endif