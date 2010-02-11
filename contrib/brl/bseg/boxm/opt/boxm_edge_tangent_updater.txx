#ifndef boxm_edge_tangent_updater_txx_
#define boxm_edge_tangent_updater_txx_

#include "boxm_edge_tangent_updater.h"

#include <boxm/boxm_apm_traits.h>
#include <boxm/opt/boxm_aux_traits.h>
#include <boxm/boxm_scene.h>
#include <boxm/opt/boxm_aux_scene.h>

#include <vgl/vgl_infinite_line_3d.h>
#include <vgl/vgl_plane_3d.h>
#include <vgl/algo/vgl_intersection.h>

#include <vcl_vector.h>
#include <vcl_string.h>

template <class T_loc, class APM, class AUX>
boxm_edge_tangent_updater<T_loc,APM,AUX>::boxm_edge_tangent_updater(boxm_scene<boct_tree<T_loc,
                                                                    boxm_inf_line_sample<APM> > > &scene,
                                                                    vcl_vector<vcl_string> const& image_ids)
: image_ids_(image_ids), scene_(scene)
{}


template <class T_loc, class APM, class AUX>
bool boxm_edge_tangent_updater<T_loc,APM,AUX>::add_cells()
{
  // get auxiliary scenes associated with each input image


  typedef boct_tree<T_loc, boxm_inf_line_sample<APM> > tree_type;
  typedef boct_tree<T_loc, boxm_edge_tangent_sample<AUX> > aux_tree_type;

  vcl_vector<boxm_aux_scene<T_loc,  boxm_inf_line_sample<APM>, boxm_edge_tangent_sample<AUX> > > aux_scenes;
  for (unsigned int i=0; i<image_ids_.size(); ++i) {
    boxm_aux_scene<T_loc, boxm_inf_line_sample<APM>, boxm_edge_tangent_sample<AUX> > aux_scene(&scene_,image_ids_[i],boxm_aux_scene<T_loc, boxm_inf_line_sample<APM>, boxm_edge_tangent_sample<AUX> >::LOAD);
    aux_scenes.push_back(aux_scene);
  }

  vcl_vector<boxm_edge_tangent_sample<APM> > aux_samples;

  // for each block
  boxm_block_iterator<tree_type> iter(&scene_);
  iter.begin();
  while (!iter.end())
  {
    scene_.load_block(iter.index());
    boxm_block<tree_type>* block = *iter;
    boct_tree<T_loc, boxm_inf_line_sample<APM> >* tree = block->get_tree();
    vcl_vector<boct_tree_cell<T_loc,boxm_inf_line_sample<APM> >*> cells = tree->leaf_cells();

    // get a vector of incremental readers for each aux scene.
    vcl_vector<boct_tree_cell_reader<T_loc, boxm_edge_tangent_sample<AUX> > *> aux_readers(aux_scenes.size());
    for (unsigned int i=0; i<aux_scenes.size(); ++i) {
      aux_readers[i] = aux_scenes[i].get_block_incremental(iter.index());
    }
    // iterate over cells
    for (unsigned i=0; i<cells.size(); ++i)
    {
      aux_samples.clear();
      boct_tree_cell<T_loc,boxm_inf_line_sample<APM> >* cell = cells[i];
#if 0
      boxm_inf_line_sample<APM> data = cell->data();
      vcl_cout << "cell IN " << data.alpha << data.appearence_<< vcl_endl;
#endif // 0
      for (unsigned j=0; j<aux_readers.size(); j++) {
        boct_tree_cell<T_loc, boxm_edge_tangent_sample<AUX> > temp_cell;

        if (!aux_readers[j]->next(temp_cell)) {
          vcl_cerr << "error: incremental reader returned false.\n";
          return false;
        }

        if (!temp_cell.code_.isequal(&(cell->code_))) {
          vcl_cerr << "error: temp_cell idx does not match cell idx.\n";
          return false;
        }
//      if (temp_cell.data().seg_len_ > 0.0f) {
          aux_samples.push_back(temp_cell.data());
//      }
      }

      vcl_list<vgl_plane_3d<AUX> > planes;
      vcl_vector<AUX> weights;
      for (unsigned int i=0; i<aux_samples.size(); ++i) {
        boxm_edge_tangent_sample<APM> s = aux_samples[i];
        for (unsigned int j=0; j<s.num_obs(); j++) {
          boxm_plane_obs<AUX> obs = s.obs(j);
          weights.push_back(obs.seg_len_);
          vgl_plane_3d<AUX> plane(obs.plane_);
          planes.push_back(plane);
        }
      }

      vgl_infinite_line_3d<AUX> line = vgl_intersection(planes, weights);
      boxm_inf_line_sample<AUX> data(line);
      cell->set_data(data);
    }
    scene_.write_active_block();
    for (unsigned int i=0; i<aux_readers.size(); ++i) {
      aux_readers[i]->close();
    }
    iter++;
 }
#ifdef DEBUG
  vcl_cout << "done with all cells" << vcl_endl;
#endif
  // clear the aux scenes so that its starts with the refined scene next time
  for (unsigned i=0; i<aux_scenes.size(); i++) {
    aux_scenes[i].clean_scene();
  }

  return true;
}


#define BOXM_EDGE_TANGENT_UPDATER_INSTANTIATE(T1,T2,T3) \
template class boxm_edge_tangent_updater<T1,T2,T3 >

#endif // boxm_edge_tangent_updater_txx_
