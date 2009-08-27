// This is brl/bseg/bvpl/bvpl_vector_operator.h
#ifndef bvpl_vector_operator_h
#define bvpl_vector_operator_h

//:
// \file
// \brief A class to apply a vector of kernels to a voxel grid
//
// \author Isabel Restrepo mir@lems.brown.edu
//
// \date  6/24/09
//
// \verbatim
//  Modifications
//   <none yet>
// \endverbatim

#include "bvpl_neighb_operator.h"
#include "bvpl_kernel_factory.h"
#include <bvpl/bvpl_local_max_functor.h>
#include <vcl_iostream.h>
#include <vcl_limits.h>


class bvpl_vector_operator
{
public:
  //: Applies a vector of kernel and functor to a grid.
  //  The result is a grid of maxima response and a grid and id to the winning kernel
  template<class T, class F>
  void apply_and_suppress(bvxm_voxel_grid<T>* grid, bvpl_kernel_vector_sptr kernel_vector,
                          bvpl_neighb_operator<T,F>* oper, bvxm_voxel_grid<T>* out_grid,
                          bvxm_voxel_grid<unsigned >* id_grid);
  template<class T>
  void non_maxima_suppression(bvxm_voxel_grid<T>* grid_in, bvxm_voxel_grid<unsigned >* id_grid,
                              bvpl_kernel_vector_sptr kernel_vector,
                              bvxm_voxel_grid<T>* out_grid);
  template<class T> 
  void filter_response(bvxm_voxel_grid<T>* grid_in, bvxm_voxel_grid<unsigned >* id_grid,
                       unsigned targed_id, bvxm_voxel_grid<float>* out_grid);

private:
  template<class T>
  void get_max_orientation_grid(bvxm_voxel_grid<T>* out_grid, bvxm_voxel_grid<T>* temp_grid,
                                bvxm_voxel_grid<unsigned >* id_grid,
                                unsigned temp_orientation);


};


//: local non-max suppression
template<class T>
void bvpl_vector_operator::non_maxima_suppression(bvxm_voxel_grid<T>* grid_in, bvxm_voxel_grid<unsigned >* id_grid,
                                                        bvpl_kernel_vector_sptr kernel_vector,
                                                        bvxm_voxel_grid<T>* grid_out)
{
  bvpl_subgrid_iterator<T> response_grid_iter(grid_in, kernel_vector->max_dim());
  bvpl_subgrid_iterator<T> out_grid_iter(grid_out, kernel_vector->max_dim());
  bvpl_subgrid_iterator<unsigned int> kernel_id_iter(id_grid, kernel_vector->max_dim());
  
  bvpl_local_max_functor<T> func_max;
  //kernel->print();
  while (!response_grid_iter.isDone()) {
    unsigned index=(*kernel_id_iter).get_voxel();
    bvpl_kernel_iterator kernel_iter = kernel_vector->kernels_[index]->iterator();
    bvpl_voxel_subgrid<T> subgrid = *response_grid_iter;
    //reset the iterator
    kernel_iter.begin();
    while (!kernel_iter.isDone()) {
      vgl_point_3d<int> idx = kernel_iter.index();
      T val;
      if (subgrid.voxel(idx, val)) {
        //vcl_cout<< val << "at " << idx <<vcl_endl;
        bvpl_kernel_dispatch d = *kernel_iter;
        func_max.apply(val, d);
      }
      ++kernel_iter;
    }
    
    // set the result at the output grid
    
    (*out_grid_iter).set_voxel(func_max.result());
    ++out_grid_iter;
    ++response_grid_iter;
    ++kernel_id_iter;
  }

    
}

template<class T, class F>
void bvpl_vector_operator::apply_and_suppress(bvxm_voxel_grid<T>* grid, 
                                                   bvpl_kernel_vector_sptr kernel_vector,
                                                   bvpl_neighb_operator<T,F>* oper, 
                                                   bvxm_voxel_grid<T>* out_grid,
                                                   bvxm_voxel_grid<unsigned>* id_grid)
{
  bvpl_local_max_functor<T> func_max;
  bvxm_voxel_grid<T> temp_grid("temp_grid.vox",grid->grid_size());
  temp_grid.initialize_data(func_max.min_response());
  out_grid->initialize_data(func_max.min_response());
  id_grid->initialize_data(0);

  for(unsigned id = 0; id < kernel_vector->kernels_.size(); ++id)
  {
    bvpl_kernel_sptr kernel = kernel_vector->kernels_[id];
    vcl_cout << "Processing axis: "  << kernel->axis() << vcl_endl;
    vcl_cout << "Processing angle: " << kernel->angle() << vcl_endl;
    vcl_cout << "Processing scale: " << kernel->scale() << vcl_endl;
    oper->operate(grid, kernel, &temp_grid);
    get_max_orientation_grid(out_grid, &temp_grid, id_grid, id);
  }
 

}

template<class T>
void bvpl_vector_operator::get_max_orientation_grid(bvxm_voxel_grid<T>* out_grid, bvxm_voxel_grid<T>* temp_grid,
                            bvxm_voxel_grid<unsigned>* id_grid,
                            unsigned id)
{
   bvpl_local_max_functor<T> func_max;
  typename bvxm_voxel_grid<T>::iterator out_grid_it = out_grid->begin();
  typename bvxm_voxel_grid<T>::iterator temp_grid_it = temp_grid->begin();
  bvxm_voxel_grid<unsigned >::iterator id_grid_it = id_grid->begin();
  
  for (; out_grid_it!=out_grid->end(); ++out_grid_it, ++temp_grid_it, ++id_grid_it)
  {
    typename bvxm_voxel_slab<T>::iterator out_slab_it = (*out_grid_it).begin();
    typename bvxm_voxel_slab<T>::iterator temp_slab_it= (*temp_grid_it).begin();
    bvxm_voxel_slab<unsigned>::iterator id_slab_it = id_grid_it->begin();

    for (; out_slab_it!=(*out_grid_it).end(); ++out_slab_it, ++temp_slab_it, ++id_slab_it)
    {
      if(func_max.greater_than((* temp_slab_it), (*out_slab_it)))
      {
        (*out_slab_it) =  (* temp_slab_it);
        (*id_slab_it) = id;
      }
    }
  }
}

template<class T> 
void  bvpl_vector_operator::filter_response(bvxm_voxel_grid<T>* grid_in, bvxm_voxel_grid<unsigned >* id_grid,
                     unsigned targed_id, bvxm_voxel_grid<float>* out_grid)
{
  bvpl_local_max_functor<T> func_max;
  bvxm_voxel_grid<float>::iterator out_grid_it = out_grid->begin();
  typename bvxm_voxel_grid<T>::iterator in_grid_it = grid_in->begin();
  bvxm_voxel_grid<unsigned >::iterator id_grid_it = id_grid->begin();
  
  for (; out_grid_it!=out_grid->end(); ++out_grid_it, ++in_grid_it, ++id_grid_it)
  {
    bvxm_voxel_slab<float>::iterator out_slab_it = (*out_grid_it).begin();
    typename bvxm_voxel_slab<T>::iterator in_slab_it= (*in_grid_it).begin();
    bvxm_voxel_slab<unsigned>::iterator id_slab_it = id_grid_it->begin();
    
    for (; out_slab_it!=(*out_grid_it).end(); ++out_slab_it, ++in_slab_it, ++id_slab_it)
    {
      *out_slab_it = func_max.filter_response(*id_slab_it, targed_id, *in_slab_it);

    }
  }
}


#endif // bvpl_vector_operator_h
