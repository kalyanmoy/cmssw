#ifndef RecoParticleFlow_PFClusterProducer_PFEcalRecHitQTests_h
#define RecoParticleFlow_PFClusterProducer_PFEcalRecHitQTests_h

#include "RecoParticleFlow/PFClusterProducer/interface/PFRecHitQTestBase.h"



//
//  Quality test that checks threshold
//
class PFRecHitQTestThreshold : public PFRecHitQTestBase {
 public:
  PFRecHitQTestThreshold() {

  }

  PFRecHitQTestThreshold(const edm::ParameterSet& iConfig):
    PFRecHitQTestBase(iConfig)
    {
      threshold_ = iConfig.getParameter<double>("threshold");

    }

    void beginEvent(const edm::Event& event,const edm::EventSetup& iSetup) {
    }

    bool test(reco::PFRecHit& hit,const EcalRecHit& rh,bool& clean) {
      return pass(hit);
    }
    bool test(reco::PFRecHit& hit,const HBHERecHit& rh,bool& clean) {
      return pass(hit);
    }

    bool test(reco::PFRecHit& hit,const HFRecHit& rh,bool& clean) {
      return pass(hit);
    }
    bool test(reco::PFRecHit& hit,const HORecHit& rh,bool& clean) {
      return pass(hit);
    }

    bool test(reco::PFRecHit& hit,const CaloTower& rh,bool& clean) {
      return pass(hit);
    }

    bool test(reco::PFRecHit& hit,const HGCRecHit& rh,bool& clean) {
      return pass(hit);
    }

 protected:
  double threshold_;

  bool pass(const reco::PFRecHit& hit) {
    if (hit.energy()>threshold_) return true;

    return false;
  }
};

class PFRecHitQTestThresholdInMIPs : public PFRecHitQTestBase {
 public:
  PFRecHitQTestThresholdInMIPs() {

  }

  PFRecHitQTestThresholdInMIPs(const edm::ParameterSet& iConfig):
    PFRecHitQTestBase(iConfig)
    {
      recHitEnergy_keV_ = iConfig.getParameter<bool>("recHitEnergyIs_keV");
      threshold_ = iConfig.getParameter<double>("thresholdInMIPs");
      mip_ = iConfig.getParameter<double>("mipValueInkeV");
      recHitEnergyMultiplier_ = iConfig.getParameter<double>("recHitEnergyMultiplier");         
    }

    void beginEvent(const edm::Event& event,const edm::EventSetup& iSetup) {
    }

    bool test(reco::PFRecHit& hit,const EcalRecHit& rh,bool& clean) {
      throw cms::Exception("WrongDetector")
	<< "PFRecHitQTestThresholdInMIPs only works for HGCAL!";
      return false;
    }
    bool test(reco::PFRecHit& hit,const HBHERecHit& rh,bool& clean) {
      throw cms::Exception("WrongDetector")
	<< "PFRecHitQTestThresholdInMIPs only works for HGCAL!";
      return false;
    }

    bool test(reco::PFRecHit& hit,const HFRecHit& rh,bool& clean) {
      throw cms::Exception("WrongDetector")
	<< "PFRecHitQTestThresholdInMIPs only works for HGCAL!";
      return false;
    }
    bool test(reco::PFRecHit& hit,const HORecHit& rh,bool& clean) {
      throw cms::Exception("WrongDetector")
	<< "PFRecHitQTestThresholdInMIPs only works for HGCAL!";
      return false;
    }

    bool test(reco::PFRecHit& hit,const CaloTower& rh,bool& clean) {
      throw cms::Exception("WrongDetector")
	<< "PFRecHitQTestThresholdInMIPs only works for HGCAL!";
      return false;
    }

    bool test(reco::PFRecHit& hit,const HGCRecHit& rh,bool& clean) {
      const double newE = ( recHitEnergy_keV_ ? 
			    1.0e-6*rh.energy()*recHitEnergyMultiplier_ :
			    rh.energy()*recHitEnergyMultiplier_ );
      /*
      std::cout << hit.position() << ' ' 
		<< rh.energy() << ' ' << hit.energy() << std::endl;
      */
      hit.setEnergy(newE);
      
      return pass(hit);
    }

 protected:
    bool recHitEnergy_keV_;
    double threshold_,mip_,recHitEnergyMultiplier_;    

  bool pass(const reco::PFRecHit& hit) {
    const double eta_correction = std::tanh(hit.position().Eta());
    // coth = 1/tanh && convert PF hit energy back in keV to compare to MIP
    const double hitValueInMIPs = 1e6*hit.energy()*eta_correction/mip_;
    //std::cout << "hit value in MIPs : " << hitValueInMIPs <<std::endl;
    return hitValueInMIPs > threshold_;
  }
};


//
//  Quality test that checks kHCAL Severity
//
class PFRecHitQTestHCALChannel : public PFRecHitQTestBase {
 public:
  PFRecHitQTestHCALChannel() {

  }

  PFRecHitQTestHCALChannel(const edm::ParameterSet& iConfig):
    PFRecHitQTestBase(iConfig)
    {
      threshold_ = iConfig.getParameter<int>("maxSeverity");
    }

    void beginEvent(const edm::Event& event,const edm::EventSetup& iSetup) {
      edm::ESHandle<HcalTopology> topo;
      iSetup.get<HcalRecNumberingRecord>().get( topo );
      edm::ESHandle<HcalChannelQuality> hcalChStatus;    
      iSetup.get<HcalChannelQualityRcd>().get( hcalChStatus );
      theHcalChStatus_ = hcalChStatus.product();
      if (!theHcalChStatus_->topo()) theHcalChStatus_->setTopo(topo.product());
      edm::ESHandle<HcalSeverityLevelComputer> hcalSevLvlComputerHndl;
      iSetup.get<HcalSeverityLevelComputerRcd>().get(hcalSevLvlComputerHndl);
      hcalSevLvlComputer_  =  hcalSevLvlComputerHndl.product();
    }

    bool test(reco::PFRecHit& hit,const EcalRecHit& rh,bool& clean) {
      return true;
    }
    bool test(reco::PFRecHit& hit,const HBHERecHit& rh,bool& clean) {
      const HcalDetId& detid = (HcalDetId)rh.detid();
      const HcalChannelStatus* theStatus = theHcalChStatus_->getValues(detid);
      unsigned theStatusValue = theStatus->getValue();
      // Now get severity of problems for the given detID, based on the rechit flag word and the channel quality status value
      int hitSeverity=hcalSevLvlComputer_->getSeverityLevel(detid, rh.flags(),theStatusValue);

      if (hitSeverity>threshold_) {
	clean=true;
	return false;
      }
      
      return true;
    }

    bool test(reco::PFRecHit& hit,const HFRecHit& rh,bool& clean) {
      const HcalDetId& detid = (HcalDetId)rh.detid();
      const HcalChannelStatus* theStatus = theHcalChStatus_->getValues(detid);
      unsigned theStatusValue = theStatus->getValue();
      // Now get severity of problems for the given detID, based on the rechit flag word and the channel quality status value
      int hitSeverity=hcalSevLvlComputer_->getSeverityLevel(detid, rh.flags(),theStatusValue);

      if (hitSeverity>threshold_) {
	clean=true;
	return false;
      }

      return true;
    }
    bool test(reco::PFRecHit& hit,const HORecHit& rh,bool& clean) {

      const HcalDetId& detid = (HcalDetId)rh.detid();
      const HcalChannelStatus* theStatus = theHcalChStatus_->getValues(detid);
      unsigned theStatusValue = theStatus->getValue();

      // Now get severity of problems for the given detID, based on the rechit flag word and the channel quality status value

      int hitSeverity=hcalSevLvlComputer_->getSeverityLevel(detid, rh.flags(),theStatusValue);

      if (hitSeverity>threshold_) {
	clean=true;
	return false;
      }

      return true;
    }

    bool test(reco::PFRecHit& hit,const CaloTower& rh,bool& clean) {
      return true;

    }

    bool test(reco::PFRecHit& hit,const HGCRecHit& rh,bool& clean) {
      return true;

    }

 protected:
  int threshold_;
  const HcalChannelQuality* theHcalChStatus_;
  const HcalSeverityLevelComputer* hcalSevLvlComputer_;
};


//
//  Quality test that checks HO threshold applying different threshold in rings
//
class PFRecHitQTestHOThreshold : public PFRecHitQTestBase {
 public:
  PFRecHitQTestHOThreshold() {

  }

  PFRecHitQTestHOThreshold(const edm::ParameterSet& iConfig):
    PFRecHitQTestBase(iConfig)
    {
      threshold0_ = iConfig.getParameter<double>("threshold_ring0");
      threshold12_ = iConfig.getParameter<double>("threshold_ring12");
    }

    void beginEvent(const edm::Event& event,const edm::EventSetup& iSetup) {
    }

    bool test(reco::PFRecHit& hit,const EcalRecHit& rh,bool& clean) {
      return true;
    }
    bool test(reco::PFRecHit& hit,const HBHERecHit& rh,bool& clean) {
      return true;
    }

    bool test(reco::PFRecHit& hit,const HFRecHit& rh,bool& clean) {
      return true;
    }
    bool test(reco::PFRecHit& hit,const HORecHit& rh,bool& clean) {
      HcalDetId detid(rh.detid());
      if (abs(detid.ieta())<=4 && hit.energy()>threshold0_)
	return true;
      if (abs(detid.ieta())>4 && hit.energy()>threshold12_)
	return true;
      
      return false;
    }

    bool test(reco::PFRecHit& hit,const CaloTower& rh,bool& clean) {
      return true;
    }

    bool test(reco::PFRecHit& hit,const HGCRecHit& rh,bool& clean) {
      return true;
    }

 protected:
  double threshold0_;
  double threshold12_;

};

//
//  Quality test that checks ecal quality cuts
//
class PFRecHitQTestECAL : public PFRecHitQTestBase {
 public:
  PFRecHitQTestECAL() {

  }

  PFRecHitQTestECAL(const edm::ParameterSet& iConfig):
    PFRecHitQTestBase(iConfig)
    {
      thresholdCleaning_   = iConfig.getParameter<double>("cleaningThreshold");
      timingCleaning_      = iConfig.getParameter<bool>("timingCleaning");
      topologicalCleaning_ = iConfig.getParameter<bool>("topologicalCleaning");
      skipTTRecoveredHits_ = iConfig.getParameter<bool>("skipTTRecoveredHits");

    }

    void beginEvent(const edm::Event& event,const edm::EventSetup& iSetup) {
    }

    bool test(reco::PFRecHit& hit,const EcalRecHit& rh,bool& clean) {
      if (skipTTRecoveredHits_ && rh.checkFlag(EcalRecHit::kTowerRecovered))
	{
	  clean=true;
	  return false;
	}
      if (  timingCleaning_ && rh.energy() > thresholdCleaning_ && 
	    rh.checkFlag(EcalRecHit::kOutOfTime) ) {
	  clean=true;
	  return false;
      }
      
      if    ( topologicalCleaning_ && 
	      ( rh.checkFlag(EcalRecHit::kWeird) || 
		rh.checkFlag(EcalRecHit::kDiWeird))) {
	clean=true;
	return false;
      }

      return true;
    }

    bool test(reco::PFRecHit& hit,const HBHERecHit& rh,bool& clean) {
      return true;
    }

    bool test(reco::PFRecHit& hit,const HFRecHit& rh,bool& clean) {
      return true;

    }

    bool test(reco::PFRecHit& hit,const HORecHit& rh,bool& clean) {
      return true;
    }

    bool test(reco::PFRecHit& hit,const CaloTower& rh,bool& clean) {
      return true;

    }
    
    bool test(reco::PFRecHit& hit,const HGCRecHit& rh,bool& clean) {
      return true;

    }

 protected:
  double thresholdCleaning_;
  bool timingCleaning_;
  bool topologicalCleaning_;
  bool skipTTRecoveredHits_;

};





//
//  Quality test that calibrates tower 29 of HCAL
//
class PFRecHitQTestHCALCalib29 : public PFRecHitQTestBase {
 public:
  PFRecHitQTestHCALCalib29() {

  }

  PFRecHitQTestHCALCalib29(const edm::ParameterSet& iConfig):
    PFRecHitQTestBase(iConfig)
    {
      calibFactor_ =iConfig.getParameter<double>("calibFactor");
    }

    void beginEvent(const edm::Event& event,const edm::EventSetup& iSetup) {
    }

    bool test(reco::PFRecHit& hit,const EcalRecHit& rh,bool& clean) {
      return true;
    }
    bool test(reco::PFRecHit& hit,const HBHERecHit& rh,bool& clean) {
      HcalDetId detId(hit.detId());
      if (abs(detId.ieta())==29)
	hit.setEnergy(hit.energy()*calibFactor_);
      return true;

    }

    bool test(reco::PFRecHit& hit,const HFRecHit& rh,bool& clean) {
      return true;

    }
    bool test(reco::PFRecHit& hit,const HORecHit& rh,bool& clean) {
      return true;
    }

    bool test(reco::PFRecHit& hit,const CaloTower& rh,bool& clean) {
      CaloTowerDetId detId(hit.detId());
      if (detId.ietaAbs()==29)
	hit.setEnergy(hit.energy()*calibFactor_);
      return true;
	  
    }

    bool test(reco::PFRecHit& hit,const HGCRecHit& rh,bool& clean) {
      return true;
    }

 protected:
    float calibFactor_;
};



#endif
