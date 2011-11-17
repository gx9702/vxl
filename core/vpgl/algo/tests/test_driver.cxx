#include <testlib/testlib_register.h>

DECLARE( test_fm_compute );
DECLARE( test_camera_compute );
DECLARE( test_camera_convert );
DECLARE( test_backproject );
DECLARE( test_project );
DECLARE( test_ray );
DECLARE( test_ray_intersect );
DECLARE( test_optimize_camera );
DECLARE( test_ortho_procrustes );
DECLARE( test_rational_adjust_onept );
DECLARE( test_rational_adjust );
DECLARE( test_em_compute_5_point );
DECLARE( test_triangulate );
DECLARE( test_bundle_adjust );
DECLARE( test_ba_fixed_k_lsqr );
DECLARE( test_ba_shared_k_lsqr );

void register_tests()
{
REGISTER( test_fm_compute );
REGISTER( test_camera_compute );
REGISTER( test_camera_convert );
REGISTER( test_project );
REGISTER( test_backproject );
REGISTER( test_ray );
REGISTER( test_ray_intersect );
REGISTER( test_optimize_camera )
REGISTER( test_ortho_procrustes );
REGISTER( test_rational_adjust_onept );
REGISTER( test_rational_adjust );
REGISTER( test_em_compute_5_point );
REGISTER( test_triangulate );
REGISTER( test_bundle_adjust );
REGISTER( test_ba_fixed_k_lsqr );
REGISTER( test_ba_shared_k_lsqr );
}

DEFINE_MAIN;
