#include "wntr_model.hpp"

void WNTRModel::get_x(double *array_out, int array_length_out)
{
  if (!is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be fixed with set_structure before get_x can be called.");
    }
  int i = 0;
  for (Var* &v_ptr : vars_vector)
    {
      array_out[i] = v_ptr->value;
      ++i;
    }
}


void WNTRModel::load_var_values_from_x(double *arrayin, int array_length_in)
{
  if (!is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be fixed with set_structure before load_var_values_from_x can be called.");
    }
  int i = 0;
  for (Var* &v_ptr: vars_vector)
    {
      v_ptr->value = arrayin[i];
      ++i;
    }
}


void WNTRModel::add_constraint(Constraint* con)
{
  if (is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be released with release_structure before add_constraint can be called.");
    }
  cons.insert(con);
  nnz += con->num_vars;
}


void WNTRModel::remove_constraint(Constraint* con)
{
  if (is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be released with release_structure before remove_constraint can be called.");
    }
  cons.erase(con);
  nnz -= con->num_vars;
}


void WNTRModel::evaluate(double *array_out, int array_length_out)
{
  if (!is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be fixed with set_structure before evaluate can be called.");
    }
  int i = 0;
  for (auto &ptr_to_con : cons_vector)
    {
      array_out[i] = ptr_to_con->evaluate();
      ++i;
    }
}


void WNTRModel::evaluate_csr_jacobian(double *values_array_out, int values_array_length_out, int *col_ndx_array_out, int col_ndx_array_length_out, int *row_nnz_array_out, int row_nnz_array_length_out)
{
  if (!is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be fixed with set_structure before evaluate_csr_jacobian can be called.");
    }
  int _row_nnz = 0;
  int _col_ndx = 0;
  int _values = 0;

  row_nnz_array_out[_row_nnz] = 0;
  ++_row_nnz;

  for (auto &con_iter : cons_vector)
    {
      row_nnz_array_out[_row_nnz] = row_nnz_array_out[_row_nnz - 1] + con_iter->num_vars;
      ++_row_nnz;
      con_iter->rad();
      for (int i=0; i<con_iter->num_vars; ++i)
	{
	  values_array_out[_values] = con_iter->vars[i]->der;
	  col_ndx_array_out[_col_ndx] = con_iter->vars[i]->index;
	  ++_values;
	  ++_col_ndx;
	}
    }
}


void WNTRModel::add_var(ExpressionBase* v)
{
  if (is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be released with release_structure before add_var can be called.");
    }
  assert (v->is_var());
  vars.insert(dynamic_cast<Var*>(v));
}


void WNTRModel::remove_var(ExpressionBase* v)
{
  if (is_structure_fixed)
    {
      throw std::runtime_error("The structure of the model must be released with release_structure before remove_var can be called.");
    }
  assert (v->is_var());
  vars.erase(dynamic_cast<Var*>(v));
}


void WNTRModel::set_structure()
{
  is_structure_fixed = true;
  
  int _v = 0;
  for (auto it = vars.begin(); it != vars.end(); ++it)
    {
      vars_vector.push_back(*it);
      (*it)->index = _v;
      ++_v;
    }

  int _c = 0;
  for (auto it = cons.begin(); it != cons.end(); ++it)
    {
      (*it)->index = _c;
      cons_vector.push_back(*it);
      ++_c;
    }
}


void WNTRModel::release_structure()
{
  is_structure_fixed = false;
  cons_vector.clear();
  vars_vector.clear();
}
