#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/HGCalCommonData/interface/HGCalWaferIndex.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometryLoader.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/FlatTrd.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "DataFormats/ForwardDetId/interface/HFNoseDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/ForwardDetId/interface/ForwardSubdetector.h"

//#define EDM_ML_DEBUG

typedef CaloCellGeometry::CCGFloat CCGFloat;
typedef std::vector<float> ParmVec;

HGCalGeometryLoader::HGCalGeometryLoader () : twoBysqrt3_(2.0/std::sqrt(3.0)) {
}

HGCalGeometryLoader::~HGCalGeometryLoader () {}

HGCalGeometry* HGCalGeometryLoader::build (const HGCalTopology& topology) {

  // allocate geometry
  HGCalGeometry* geom = new HGCalGeometry (topology);
  unsigned int numberOfCells = topology.totalGeomModules(); // both sides
  unsigned int numberExpected= topology.allGeomModules();
  HGCalGeometryMode::GeometryMode mode = topology.geomMode();
  parametersPerShape_ = ((mode == HGCalGeometryMode::Trapezoid) ?
			 (int)HGCalGeometry::k_NumberOfParametersPerTrd :
			 (int)HGCalGeometry::k_NumberOfParametersPerHex);
  uint32_t numberOfShapes = ((mode == HGCalGeometryMode::Trapezoid) ?
			     HGCalGeometry::k_NumberOfShapesTrd :
			     HGCalGeometry::k_NumberOfShapes);
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "Number of Cells " << numberOfCells << ":" 
				<< numberExpected << " for sub-detector " 
				<< topology.subDetector() << " Shapes " 
				<< numberOfShapes << ":" << parametersPerShape_
				<< " mode " << mode;
#endif
  geom->allocateCorners( numberOfCells ) ;
  geom->allocatePar(numberOfShapes, parametersPerShape_);

  // loop over modules
  ParmVec params(parametersPerShape_,0);
  unsigned int counter(0);
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "HGCalGeometryLoader with # of "
				<< "transformation matrices " 
				<< topology.dddConstants().getTrFormN()
				<< " and " << topology.dddConstants().volumes()
				<< ":" << topology.dddConstants().sectors() 
				<< " volumes";
#endif
  for (unsigned itr=0; itr<topology.dddConstants().getTrFormN(); ++itr) {
    HGCalParameters::hgtrform mytr = topology.dddConstants().getTrForm(itr);
    int zside  = mytr.zp;
    int layer  = mytr.lay;
#ifdef EDM_ML_DEBUG
    unsigned int kount(0);
    edm::LogVerbatim("HGCalGeom") << "HGCalGeometryLoader:: Z:Layer " << zside
				  << ":" << layer << " z " << mytr.h3v.z();
#endif
    if ((mode == HGCalGeometryMode::Hexagon) || 
	(mode == HGCalGeometryMode::HexagonFull)) {
      ForwardSubdetector subdet  = topology.subDetector();
      for (int wafer=0; wafer<topology.dddConstants().sectors(); ++wafer) {
	std::string code[2] = {"False", "True"};
	if (topology.dddConstants().waferInLayer(wafer,layer,true)) {
	  int type = topology.dddConstants().waferTypeT(wafer);
	  if (type != 1) type = 0;
	  DetId detId = (DetId)(HGCalDetId(subdet,zside,layer,type,wafer,0));
	  const auto &  w = topology.dddConstants().waferPosition(wafer,true);
	  double xx = (zside > 0) ? w.first : -w.first;
	  CLHEP::Hep3Vector h3v(xx,w.second,mytr.h3v.z());
	  const HepGeom::Transform3D ht3d (mytr.hr, h3v);
#ifdef EDM_ML_DEBUG
	  edm::LogVerbatim("HGCalGeom") << "HGCalGeometryLoader:: Wafer:Type "
					<< wafer << ":" << type << " DetId " 
					<< HGCalDetId(detId) << std::hex
					<< " " << detId.rawId() << std::dec 
					<< " transf " << ht3d.getTranslation()
					<< " and " << ht3d.getRotation();
#endif
	  HGCalParameters::hgtrap vol = topology.dddConstants().getModule(wafer,true,true);
	  params[0] = vol.dz;
	  params[1] = topology.dddConstants().cellSizeHex(type);
	  params[2] = twoBysqrt3_*params[1];

	  buildGeom(params, ht3d, detId, geom, 0);
	  counter++;
#ifdef EDM_ML_DEBUG
	  ++kount;
#endif
	}
      }
    } else if (mode == HGCalGeometryMode::Trapezoid) {
      int indx = topology.dddConstants().layerIndex(layer,true);
      int ieta = topology.dddConstants().getParameter()->iEtaMinBH_[indx];
      int nphi = topology.dddConstants().getParameter()->nPhiBinBH_[indx];
      int type = topology.dddConstants().getTypeTrap(layer);
      for (int md=topology.dddConstants().getParameter()->firstModule_[indx];
	   md<=topology.dddConstants().getParameter()->lastModule_[indx];++md){
	for (int iphi=1; iphi<=nphi; ++iphi) {
	  DetId detId = (DetId)(HGCScintillatorDetId(type,layer,zside*ieta,iphi));
	  const auto &  w = topology.dddConstants().locateCellTrap(layer,ieta,iphi,true);
	  double xx = (zside > 0) ? w.first : -w.first;
	  CLHEP::Hep3Vector h3v(xx,w.second,mytr.h3v.z());
	  const HepGeom::Transform3D ht3d (mytr.hr, h3v);
#ifdef EDM_ML_DEBUG
	  edm::LogVerbatim("HGCalGeom") << "HGCalGeometryLoader::eta:phi:type "
					<< ieta*zside << ":" << iphi << ":"
					<< type << " DetId " 
					<< HGCScintillatorDetId(detId) << " "
					<< std::hex << detId.rawId() 
					<< std::dec << " transf " 
					<< ht3d.getTranslation() << " and "
					<< ht3d.getRotation();
#endif
	  HGCalParameters::hgtrap vol = topology.dddConstants().getModule(md,false,true);
	  params[0] = vol.dz;
	  params[1] = params[2] = 0;
	  params[3] = params[7] = vol.h;
	  params[4] = params[8] = vol.bl;
	  params[5] = params[9] = vol.tl;
	  params[6] = params[10]= 0;
	  params[11]= topology.dddConstants().cellSizeHex(type);

	  buildGeom(params, ht3d, detId, geom, 1);
	  counter++;
#ifdef EDM_ML_DEBUG
	  ++kount;
#endif
	}
	++ieta;
      }
    } else {
      DetId::Detector det  = topology.detector();
      for (int wafer=0; wafer<topology.dddConstants().sectors(); ++wafer) {
	if (topology.dddConstants().waferInLayer(wafer,layer,true)) {
	  int copy = topology.dddConstants().getParameter()->waferCopy_[wafer];
	  int u    = HGCalWaferIndex::waferU(copy);
	  int v    = HGCalWaferIndex::waferV(copy);
	  int type = topology.dddConstants().getTypeHex(layer,u,v);
	  DetId detId = (topology.isHFNose() ? 
			 (DetId)(HFNoseDetId(zside,type,layer,u,v,0,0)) :
			 (DetId)(HGCSiliconDetId(det,zside,type,layer,u,v,0,0)));
	  const auto &  w = topology.dddConstants().waferPosition(u,v,true);
	  double xx = (zside > 0) ? w.first : -w.first;
	  CLHEP::Hep3Vector h3v(xx,w.second,mytr.h3v.z());
	  const HepGeom::Transform3D ht3d (mytr.hr, h3v);
#ifdef EDM_ML_DEBUG
	  if (topology.isHFNose()) 
	    edm::LogVerbatim("HGCalGeom") << "HGCalGeometryLoader::Wafer:Type "
					  << wafer << ":" << type << " DetId " 
					  << HFNoseDetId(detId) << std::hex
					  << " " << detId.rawId() << std::dec 
					  << " trans " << ht3d.getTranslation()
					  << " and " << ht3d.getRotation();
	  else
	    edm::LogVerbatim("HGCalGeom") << "HGCalGeometryLoader::Wafer:Type "
					  << wafer << ":" << type << " DetId " 
					  << HGCSiliconDetId(detId) << std::hex
					  << " " << detId.rawId() << std::dec 
					  << " trans " << ht3d.getTranslation()
					  << " and " << ht3d.getRotation();
#endif
	  HGCalParameters::hgtrap vol = topology.dddConstants().getModule(type,false,true);
	  params[0] = vol.dz;
	  params[1] = topology.dddConstants().cellSizeHex(type);
	  params[2] = twoBysqrt3_*params[1];

	  buildGeom(params, ht3d, detId, geom, 0);
	  counter++;
#ifdef EDM_ML_DEBUG
	  ++kount;
#endif
	}
      }
    }
#ifdef EDM_ML_DEBUG
    edm::LogVerbatim("HGCalGeom") << kount << " modules found in Layer " 
				  << layer << " Z " << zside;
#endif
  }

  geom->sortDetIds();

  if (counter != numberExpected) {
    edm::LogError("HGCalGeom") << "Inconsistent # of cells: expected " 
			       << numberExpected << ":" << numberOfCells
			       << " , inited " << counter;
    assert( counter == numberExpected ) ;
  }

  return geom;
}

void HGCalGeometryLoader::buildGeom(const ParmVec& params, 
				    const HepGeom::Transform3D& ht3d, 
				    const DetId& detId,  HGCalGeometry* geom,
				    int mode) {

#ifdef EDM_ML_DEBUG
  for (int i=0; i<parametersPerShape_; ++i) 
    edm::LogVerbatim("HGCalGeom") << "Parameter[" << i << "] : " << params[i];
#endif
  if (mode == 1) {
    std::vector<GlobalPoint> corners (FlatTrd::ncorner_);
	
    FlatTrd::createCorners( params, ht3d, corners ) ;
	
    const CCGFloat* parmPtr (CaloCellGeometry::getParmPtr(params, 
							  geom->parMgr(), 
							  geom->parVecVec()));
	
    GlobalPoint front ( 0.25 * ( corners[0].x() + 
				 corners[1].x() + 
				 corners[2].x() + 
				 corners[3].x() ),
			0.25 * ( corners[0].y() + 
				 corners[1].y() + 
				 corners[2].y() + 
				 corners[3].y() ),
			0.25 * ( corners[0].z() + 
				 corners[1].z() + 
				 corners[2].z() + 
				 corners[3].z() ) ) ;
    
    GlobalPoint back  ( 0.25 * ( corners[4].x() + 
				 corners[5].x() + 
				 corners[6].x() + 
				 corners[7].x() ),
			0.25 * ( corners[4].y() + 
				 corners[5].y() + 
				 corners[6].y() + 
				 corners[7].y() ),
			0.25 * ( corners[4].z() + 
				 corners[5].z() + 
				 corners[6].z() + 
				 corners[7].z() ) ) ;
    
    if (front.mag2() > back.mag2()) { // front should always point to the center, so swap front and back
      std::swap (front, back);
      std::swap_ranges (corners.begin(),corners.begin()+FlatTrd::ncornerBy2_,
			corners.begin()+FlatTrd::ncornerBy2_); 
    }
    geom->newCell(front, back, corners[0], parmPtr, detId) ;
  } else {
    std::vector<GlobalPoint> corners (FlatHexagon::ncorner_);
	
    FlatHexagon::createCorners( params, ht3d, corners ) ;
	
    const CCGFloat* parmPtr (CaloCellGeometry::getParmPtr(params, 
							  geom->parMgr(), 
							  geom->parVecVec()));
	
    GlobalPoint front ( FlatHexagon::oneBySix_*( corners[0].x() + 
						 corners[1].x() + 
						 corners[2].x() + 
						 corners[3].x() + 
						 corners[4].x() + 
						 corners[5].x()   ),
			FlatHexagon::oneBySix_*( corners[0].y() + 
						 corners[1].y() + 
						 corners[2].y() + 
						 corners[3].y() + 
						 corners[4].y() + 
						 corners[5].y()   ),
			FlatHexagon::oneBySix_*( corners[0].z() + 
						 corners[1].z() + 
						 corners[2].z() + 
						 corners[3].z() + 
						 corners[4].z() + 
						 corners[5].z()   ) ) ;
    
    GlobalPoint back  ( FlatHexagon::oneBySix_*( corners[6].x() + 
						 corners[7].x() + 
						 corners[8].x() + 
						 corners[9].x() + 
						 corners[10].x()+ 
						 corners[11].x()  ),
			FlatHexagon::oneBySix_*( corners[6].y() + 
						 corners[7].y() + 
						 corners[8].y() + 
						 corners[9].y() + 
						 corners[10].y()+ 
						 corners[11].y()  ),
			FlatHexagon::oneBySix_*( corners[6].z() + 
						 corners[7].z() + 
						 corners[8].z() + 
						 corners[9].z() + 
						 corners[10].z()+ 
						 corners[11].z()  ) ) ;
  
    if (front.mag2() > back.mag2()) { // front should always point to the center, so swap front and back
      std::swap (front, back);
      std::swap_ranges (corners.begin(),corners.begin()+FlatHexagon::ncornerBy2_,
			corners.begin()+FlatHexagon::ncornerBy2_); 
    }
    geom->newCell(front, back, corners[0], parmPtr, detId) ;
  }
}
