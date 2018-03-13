#ifndef MAPPROJECTIONS_H
#define MAPPROJECTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: MapProjections.h - Declares functions for projecting map latitude
 *          longitude coordinates to x and y.
 * NOTES:   Required source-code control string:
 *          "@(#)MapProjections.h	2.2 /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.MapProjections.h 11/09/99 14:03:07"
 * HISTORY: 06/95, Todd Plessel, EPA/MMTSI, Created stubs.
 *          09/95, Mark Bolstad, EPA/MMTSI, Implemented.
 *****************************************************************************/

/*=============================== MACROS ===================================*/

/* Types of map projections: */

enum { ALBERS_EQUAL_AREA,
       AZIMUTHAL_EQUIDISTANT,
       AIRY,
       AITOFF,
       MOD_ALASKA,
       APIAN_GLOBULAR_I,
       AUGUST_EPICYCLOIDAL,
       BACON_GLOBULAR,
       BIPOLAR_CONIC,
       BOGGS_EUMORPHIC,
       BONNE,
       CASSINI,
       CENTRAL_CYLINDRICAL,
       EQUAL_AREA_CYLINDRICAL,
       CHAMBERLIN_TRIMETRIC,
       COLLIGNON,
       CRASTER_PARABOLIC,
       DENOYER_SEMI_ELLIPTICAL,
       ECKERT_I,
       ECKERT_II,
       ECKERT_III,
       ECKERT_IV,
       ECKERT_V,
       ECKERT_VI,
       EQUIDISTANT_CYLINDRICAL,
       EQUIDISTANT_CONIC,
       EULER,
       FAHEY,
       FOUCAUT,
       FOUCAUT_SINUSOIDAL,
       GALL,
       GINSBURG_VIII,
       GENERAL_SINUSOIDAL_SERIES,
       GNOMONIC,
       GOODE_HOMOLOSINE,
       MOD_48_US,
       MOD_50_US,
       HAMMER,
       HATANO_ASYMMETRICAL_EQUAL_AREA,
       INTL_POLYCONIC,
       KAVRAISKY_V,
       KAVRAISKY_VII,
/*       LABORDE,  */
       LAMBERT_AZIMUTHAL_EQUAL_AREA,
       LAGRANGE,
       LARRIVEE,
       LASKOWSKI,
       LAMBERT_CONFORMAL_CONIC,
       LAMBERT_EQUAL_AREA_CONIC,
       LEE_OBLATED_STEREOGRAPHIC,
       LOXIMUTHAL,
       SPACE_OBLIQUE_FOR_LANDSAT,
       MCBRYDE_THOMAS_FLAT_POLAR_SINE__1,
       MCBRYDE_THOMAS_FLAT_POLE_SINE__2,
       MCBRIDE_THOMAS_FLAT_POLAR_PARABOLIC,
       MCBRYDE_THOMAS_FLAT_POLAR_QUARTIC,
       MCBRYDE_THOMAS_FLAT_POLAR_SINUSOIDAL,
       MERCATOR,
       MILLER_OBLATED_STEREOGRAPHIC,
       MILLER_CYLINDRICAL,
/*     MODIFIED_POLYCONIC, Not implemented in proj library */
       MOLLWEIDE,
       MURDOCH_I,
       MURDOCH_II,
       MURDOCH_III,
       NELL,
       NELL_HAMMER,
       NICOLOSI_GLOBULAR,
/*       NEAR_SIDED_PERSPECTIVE, */
/*       NEW_ZEALAND_MAP_GRID, */
       OBLIQUE_CYLINDRICAL_EQUAL_AREA,
       OBLATED_EQUAL_AREA,
       OBLIQUE_MERCATOR,
       ORTELIUS_OVAL,
       ORTHOGRAPHIC,
       PERSPECTIVE_CONIC,
       POLYCONIC,
       PUTNINS_P1,
       PUTNINS_P2,
       PUTNINS_P3,
       PUTNINS_P3x,
       PUTNINS_P4x,
       PUTNINS_P5,
       PUTNINS_P5x,
       PUTNINS_P6,
       PUTNINS_P6x,
       QUARTIC_AUTHALIC,
       ROBINSON,
       RECTANGULAR_POLYCONIC,
       SINUSOIDAL,
       STEREOGRAPHIC,
       TRANSVERSE_CENTRAL_CYLINDRICAL,
       TRANSVERSE_CYLINDRICAL_EQUAL_AREA,
       TISSOT,
       TRANSVERSE_MERCATOR,
       TWO_POINT_EQUIDISTANT,
/*       TILTED_PERSPECTIVE, */
       UNIVERSAL_POLAR_STEREOGRAPHIC,
       URMAEV_V,
       URMAEV_FLAT_POLAR_SINUSOIDAL,
       UNIVERSAL_TRANSVERSE_MERCATOR,
       VAN_DER_GRINTEN_I,
       VAN_DER_GRINTEN_II,
       VAN_DER_GRINTEN_III,
       VAN_DER_GRINTEN_IV,
       VITKOVSKY_I,
       WAGNER_I,
       WAGNER_II,
       WAGNER_III,
       WAGNER_IV,
       WAGNER_V,
       WAGNER_VI,
       WAGNER_VII,
       WERENSKIOLD_I,
       WINKEL_I,
       WINKEL_II,
       WINKEL_TRIPEL,
       LAT_LON,
       NUMBER_OF_PROJECTIONS };

enum { CUSTOM_ELLIPSE,
       MERIT_1983,
       SGS_85,
       GRS_1980,
       IAU_1976,
       AIRY_1830,
       APPL_PHYSICS_1965,
       NAVAL_WEP_LAB_1965,
       MODIFIED_AIRY,
       ANDRAE_1876,
       AUST_NATL_1969,
       GRS_67,
       BESSEL_1841,
       BESSEL_NAM_1841,
       CLARKE_1866,
       CLARKE_1880,
       COMM_MESURES_1799,
       DELAMBRE_1810,
       ENGELIS_1985,
       EVEREST_1830,
       EVEREST_1948,
       EVEREST_1956,
       EVEREST_1969,
       EVEREST,
       FISCHER_1960,
       MOD_FISCHER_1960,
       FISCHER_1968,
       HELMERT_1906,
       HOUGH,
       KNTL_1909,
       KRASSOVSKY_1942,
       KAULA_1961,
       LERCH_1979,
       MAUPERTIUS_1738,
       NEW_INTL_1967,
       PLESSIS_1817,
       SOUTHEAST_ASIA,
       WALBECK,
       WGS_60,
       WGS_66,
       WGS_72,
       WGS_84,
       NUMBER_OF_ELLIPSES };

enum { AZIMUTH,
       CENTRAL_LON,
       CENTRAL_MERIDIAN,
       CENTRAL_PARALLEL,
       CHAMB_LATITUDE,
       CHAMB_LONGITUDE,
       LAT_OF_TRUE_SCALE,
       LOTSA,
       BIG_M_FACTOR,
       MAX_LATITUDE,
       MAX_LONGITUDE,
       MERIDIAN_RATIO,
       MIN_LATITUDE,
       MIN_LONGITUDE,
       NO_ROTATION,
       NO_U_OFFSET,
       ONE_HEMISPHERE,
       PATH_NUMBER,
       ROTATION_CONVERSION,
       SATELLITE_NUMBER,
       SOUTHERN_HEMI,
       THETA,
       TILT,
       VIEW_PT_HEIGHT,
       ZONE,
       LITTLE_M_FACTOR,
       N_FACTOR,
       Q_FACTOR,
       NUMBER_OF_PARAMETERS };

enum { A, B, RF, ES, NUMBER_OF_ELL_PARAMETERS };

enum { E2S_NONE,          /* Do not convert to a sphere */
       E2S_AREA,          /* Convert to a sphere with equiv surface area */
       E2S_VOLUME,        /*  "         "             "          volume */
       E2S_ARITH_MEAN,    /* Use the arithmetic mean of the major/minor axes */
       E2S_GEOM_MEAN,     /* Use the geometic mean of the major / minor axes */
       E2S_HARMONIC_MEAN, /* Use the harmonic mean of the major / minor axes */
       NUMBER_OF_E2S_PARAMETERS };
  
/* For corners[ LOWER | UPPER ][ LAT | LON ]: */

#ifndef LOWER_UPPER_LAT_LON
#define LOWER_UPPER_LAT_LON
enum { LOWER, UPPER };
enum { LAT,   LON   };
#endif

/*================================== TYPES ==================================*/

typedef struct
{
  int    type;             /* projection type: ALBERS_EQUAL_AREA, etc. */
  int    ellipse;                                 /* WGS_84, etc.            */
  int    sphereType;       /* Turn the specified ellipse into a sphere.      */
  double ellParameters[NUMBER_OF_ELL_PARAMETERS]; /* A, B, RF, ES.           */
  double corners[2][2];    /* corners[ LOWER | UPPER ][ LAT | LON ].         */
  int    maxSubdivision;   /* Maximum number of subdivisions. Will add upto  */
                           /* 2^maxSubdivision points per pair of verticies. */
  int    whichParameterSet;/* Some projections have alternate parameter sets */
  double parameters[NUMBER_OF_PARAMETERS]; /* Projection-specific parameters */
} MapProjection;

/*================================ FUNCTIONS ================================*/

extern int  isValidMapProjection( const MapProjection* mapProjection );
extern int  isMapProjectionInvertible( const MapProjection* mapProjection );
extern int  setMapProjection(     const MapProjection* mapProjection );
extern void projectLatLon( double lat, double lon, double* x, double* y );
extern void projectXY( double x, double y, double* lat, double* lon );
extern int  currentProjectionIsSet( void );
extern const MapProjection* getCurrentProjection( void );

#ifdef __cplusplus
}
#endif

#endif /* MAPPROJECTIONS_H */

