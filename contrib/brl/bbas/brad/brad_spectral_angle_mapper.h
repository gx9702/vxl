// This is brl/bbas/brad/brad_ms_aster.h
#ifndef brad_spectral_angle_mapper_h
#define brad_spectral_angle_mapper_h

#include <vector>
#include <vil/vil_image_view.h>
#include <brad/brad_multispectral_functions.h>

//:
// \file
// \brief Compares image pixel spectra to spectral library obtained from ASTER files or sampled from an image
// \author Tom Pollard and Selene Chew
// \date April 7, 2017

class brad_spectral_angle_mapper {
public:
  //: Constructor
  brad_spectral_angle_mapper(std::vector<float>& bands_min,
    std::vector<float>& bands_max);

  //: Compute the max spectral angle map for all spectra whose name includes keyword
  bool compute_sam_img(const vil_image_view<float>& image,  // image to compare to materials in the library
    const std::string keyword,                              // material of interest
    vil_image_view<float>& spectral_angle);                 // spectral angle map

  //: Compare image to a specific spectrum by taking the sum of squared differences
  //bool compute_sdm_img(const vil_image_view<float>& image,  // image to compare to spectrum of material in library
  //  const std::string keyword,                              // material of interest
  //  vil_image_view<float>& spectral_diff);                  // spectral difference map

  //: Port of Cara's matClass.m which will use a fair amount of memory
  // in class_img value -1 corresponds to unknown category (when no category meets threshold)
  // other values correspond to index of keyword in keywords
  bool aster_classify_material(
    const vil_image_view<float>& image,            // image to compare to materials in the library
    const std::vector<std::string>& keywords,      // list of materials of interest
    const float threshold,                         // [0,1] threshold that must be surpassed in order for pixel to be classified
    vil_image_view<int>& class_img,                // class of each pixel (corresponding to index of keywords; -1 indicates no class met the threshold)
    vil_image_view<float>& conf_img);              // maximum spectral angle score at each pixel

  //: add a new material to the library from labelled image pixels by averaging all pixel samples
  bool add_material(const std::string type,       // material name; will be addded to file_names_
    const vil_image_view<float>& image,           // image that pixels will be sampled from to create a new spectrum
    const vil_image_view<bool>& mask);            // new spectrum will be generated by averaging spectra of true pixels in mask

  //: add a new material to the library from labelled image pixels with each labelled pixel being its own spectrum in library
  bool add_material_per_pixel(const std::string type,       // material name; will be addded to file_names_
    const vil_image_view<float>& image,           // image that pixels will be sampled from to create a new spectrum
    const vil_image_view<bool>& mask);            // new spectrum for each true pixel in mask

  //: Find all files that only contain valid characters and add their spectra to library
  void add_aster_dir(const std::string aster_dir); // name of the directory containing aster files to be added to library

  //: Clear all materials from the library
  void clear_library();                           // empty the library (file_names_ and spectra_)

protected:
  std::vector<std::string> file_names_; // list of all materials
  std::vector<std::vector<float> > spectra_; // normalized spectra corresponding to materials in file_names_
  std::vector<float> bands_min_; // defines the lower bound of each band
  std::vector<float> bands_max_; // defines the upper bound of each band


  //: Parse a single ASTER file to retrieve wavelength/spectra data and
  // interpolate spectra for the given bands.
  bool parse_aster_file(const std::string& file_name, // aster file name
    std::vector<float>& sample_spectra);              // spectra obtained after parsing file_name

  //: Compute spectral angle between two vectors of multi-spectral values
  // as per the spectral angle mapper algorithm for a set of spectra samples.
  void compute_spectral_angles(const std::vector<float>& img_vals,                       // un-normalized image spectrum
    const std::vector<std::vector<std::vector<float> > >& normalized_spectra_samples,    // vector of vectors of spectra with each entry corresponding to a material
    std::vector<float>& angles);                                                         // vector of angle dot products each corresponding to a material in normalized_spectra_samples

  //: Compute spectral difference (sum of squared difference) between two vectors
  //  of multi-spectral values
  //void compute_spectral_diffs(
  //  const std::vector<float>& img_vals,                                                   // un-normalized image spectrum
  //  const std::vector<std::vector<std::vector<float> > >& normalized_spectra_samples,     // vector of vectors of spectra with each entry corresponding to a material
  //  std::vector<float>& differences);                                                     // vector of angle differences each corresponding to a material in normalized_spectra_samples
};


// Compute the spectral angle between two pre-normalized spectra
float brad_compute_spectral_angle(
  const float* norm_spectra1,
  const float* norm_spectra2,
  int num_channels);

// Compute the cosine of the spectral angel between two pre-normalized spectra
// note: no checks to excluded invalid aster file bands
float brad_compute_cos_spectral_angle(
  const float* norm_spectra1,
  const float* norm_spectra2,
  int num_channels);


#endif // brad_spectral_angle_mapper_h
