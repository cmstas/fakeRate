#include "myBabyMaker.h"

 // C++ includes
#include <iostream>
#include <fstream>
#include <set>
#include <exception>
#include <string>

// ROOT includes
#include "TSystem.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TChainElement.h"
#include "TH1F.h"
#include "TH2F.h"
#include "Math/VectorUtil.h"
#include "TObjArray.h"
#include "TString.h"
#include "TVector2.h"
#include "TDatabasePDG.h"
#include "TBenchmark.h"

// TAS includes
// This is for those using a makefile 
// for linking CORE and Tools as a standalone librabry 
// you need to define __NON_ROOT_BUILD__ in your build script
// (i.e. g++ ... -D__NON_ROOT_BUILD__ ... )
#ifdef __NON_ROOT_BUILD__  
#include "CMS2.h"
#include "CORE/utilities.h"
#include "CORE/electronSelections.h"
#include "CORE/electronSelectionsParameters.h"
#include "CORE/eventSelections.h"
#include "CORE/jetSelections.h"
#include "CORE/metSelections.h"
#include "CORE/MITConversionUtilities.h"
#include "CORE/muonSelections.h"
#include "CORE/trackSelections.h"
#include "CORE/triggerUtils.h"
#include "Tools/goodrun.h"
#include "CORE/mcSelections.h"
#include "CORE/ssSelections.h"
#include "CORE/susySelections.h"
// for compiling in ACLiC (.L myBabyMaker.c++ method)
// since the source files are included
#else
#include "CMS2.cc"  
#ifndef __CINT__
#include "../CORE/utilities.cc"
#include "../CORE/electronSelections.cc"
#include "../CORE/electronSelectionsParameters.cc"
#include "../CORE/eventSelections.cc"
#include "../CORE/jetSelections.cc"
#include "../CORE/metSelections.cc"
#include "../CORE/MITConversionUtilities.cc"
#include "../CORE/muonSelections.cc"
#include "../CORE/trackSelections.cc"
#include "../CORE/triggerUtils.cc"
#include "../Tools/goodrun.cc"
#include "../CORE/mcSelections.cc"
#include "../CORE/ssSelections.cc"
#include "../CORE/susySelections.cc"
#endif // __CINT__
#endif // __NON_ROOT_BUILD__

// namespaces
using namespace std;
using namespace tas;

#ifndef __CINT__
bool header1 = false;
bool header2 = false;

void PrintTriggerDebugHeader(string outfileName){

    int width  = 7;
    ofstream outfile( Form("triggerStudy/%s", outfileName.c_str() ), ios::app );
    outfile  
        <<  setw(width) << "itrg"
        <<  setw(width) << "id"
        <<  setw(width) << "match"
        <<  setw(width) << "matchId"
        <<  setw(width) << "dr"
        <<  setw(width) << "lep pt"
        <<  setw(width) << "trg pt"
        <<  setw(width) << "lep eta"
        <<  setw(width) << "trg eta"
        <<  setw(width) << "lep phi"
        <<  setw(width) << "trg phi" 
        << "\t"         << "trigString" << endl << endl;
    outfile.close();
}

void PrintTriggerDebugLine
    (
        int itrg, 
        int id, 
        bool match, 
        bool matchId, 
        double dr, 
        const LorentzVector& lepton_p4, 
        const LorentzVector& p4tr, 
        const string& trigString, 
        int nTrig, 
        const string& outfileName
    )
{
    ofstream outfile( Form("triggerStudy/%s", outfileName.c_str() ), ios::app );

    int precis = 2;
    int width  = 7;
    outfile.setf( ios::fixed, ios::floatfield );
    outfile << setprecision(precis) << setw(width) << setfill(' ') << itrg
            << setprecision(precis) << setw(width) << setfill(' ') << id
            << setprecision(precis) << setw(width) << setfill(' ') << match
            << setprecision(precis) << setw(width) << setfill(' ') << matchId
            << setprecision(precis) << setw(width) << setfill(' ') << dr
            << setprecision(precis) << setw(width) << setfill(' ') << lepton_p4.pt()
            << setprecision(precis) << setw(width) << setfill(' ') << p4tr.pt()
            << setprecision(precis) << setw(width) << setfill(' ') << lepton_p4.eta()
            << setprecision(precis) << setw(width) << setfill(' ') << p4tr.eta()
            << setprecision(precis) << setw(width) << setfill(' ') << lepton_p4.phi()
            << setprecision(precis) << setw(width) << setfill(' ') << p4tr.phi()
            << "\t" << trigString << endl;
    if( itrg == nTrig-1 ) outfile << endl;
    outfile.close();
}

bool found_ele8 = false;
bool found_ele8_CaloIdL_TrkIdVL = false;
bool found_ele8_CaloIdL_CaloIsoVL = false;
bool found_ele17_CaloIdL_CaloIsoVL = false;
bool found_ele8_CaloIdL_CaloIsoVL_Jet40 = false;

// function for dR matching offline letpon to trigger object 
pair<int, float> TriggerMatch( LorentzVector lepton_p4, const char* trigString, double dR_cut = 0.4, int pid = 11 )
{
    float dR_min = numeric_limits<float>::max();
    dR_min = 99.0;
    int nTrig = nHLTObjects(trigString);
    if (nTrig > 0) {
        bool match   = false;
        bool matchId = false;

        for (int itrg=0; itrg<nTrig; itrg++) 
        {
            LorentzVector p4tr = p4HLTObject( trigString, itrg );
            int id             = idHLTObject( trigString, itrg );
            double dr = ROOT::Math::VectorUtil::DeltaR( lepton_p4, p4tr);
            if ( dr < dR_cut ){
                match = true;
                if( abs(id) == abs(pid) ) matchId = true;
            }
            if (dr < dR_min) dR_min = dr;


            //////////////////////////
            // Debug Mixed Triggers //
            //////////////////////////
            vector<string> triggers;
            triggers.push_back("HLT_Ele8");
            triggers.push_back("HLT_Ele8_CaloIdL_TrkIdVL");
            triggers.push_back("HLT_Ele8_CaloIdL_CaloIsoVL");
            triggers.push_back("HLT_Ele17_CaloIdL_CaloIsoVL");
            triggers.push_back("HLT_Ele8_CaloIdL_CaloIsoVL_Jet40");
            for (unsigned int i=0; i < triggers.size(); i++) {
                if (
                    ( strcmp( trigString, Form( "%s_%s", triggers.at(i).c_str(), "v1" ) ) == 0 ) || 
                    ( strcmp( trigString, Form( "%s_%s", triggers.at(i).c_str(), "v2" ) ) == 0 )
                    ) {

                    //
                    if(
                        ( strcmp( trigString, "HLT_Ele8_v1") == 0 ) || 
                        ( strcmp( trigString, "HLT_Ele8_v2") == 0 )
                        ){
                        if(found_ele8 == false){
                            found_ele8 = true;
                            PrintTriggerDebugHeader( Form("%s.txt",triggers.at(i).c_str()) );
                        }
                    }

                    //
                    if(
                        ( strcmp( trigString, "HLT_Ele8_CaloIdL_TrkIdVL_v1") == 0 ) || 
                        ( strcmp( trigString, "HLT_Ele8_CaloIdL_TrkIdVL_v2") == 0 )
                        ){
                        if(found_ele8_CaloIdL_TrkIdVL == false){
                            found_ele8_CaloIdL_TrkIdVL = true;
                            PrintTriggerDebugHeader( Form("%s.txt",triggers.at(i).c_str()) );
                        }
                    }

                    //
                    if(
                        ( strcmp( trigString, "HLT_Ele8_CaloIdL_CaloIsoVL_v1") == 0 ) || 
                        ( strcmp( trigString, "HLT_Ele8_CaloIdL_CaloIsoVL_v2") == 0 )
                        ){
                        if(found_ele8_CaloIdL_CaloIsoVL == false){
                            found_ele8_CaloIdL_CaloIsoVL = true;
                            PrintTriggerDebugHeader( Form("%s.txt",triggers.at(i).c_str()) );
                        }
                    }

                    //
                    if(
                        ( strcmp( trigString, "HLT_Ele17_CaloIdL_CaloIsoVL_v1") == 0 ) ||
                        ( strcmp( trigString, "HLT_Ele17_CaloIdL_CaloIsoVL_v2") == 0 )
                        ){
                        if(found_ele17_CaloIdL_CaloIsoVL == false){
                            found_ele17_CaloIdL_CaloIsoVL = true;
                            PrintTriggerDebugHeader( Form("%s.txt",triggers.at(i).c_str()) );
                        }
                    }

                    //
                    if(
                        ( strcmp( trigString, "HLT_Ele8_CaloIdL_CaloIsoVL_Jet40_v1") == 0 ) || 
                        ( strcmp( trigString, "HLT_Ele8_CaloIdL_CaloIsoVL_Jet40_v2") == 0 )
                        ){
                        if(found_ele8_CaloIdL_CaloIsoVL_Jet40 == false){
                            found_ele8_CaloIdL_CaloIsoVL_Jet40 = true;
                            PrintTriggerDebugHeader( Form("%s.txt",triggers.at(i).c_str()) );
                        }
                    }

                    PrintTriggerDebugLine( itrg, id, match, matchId, dr, lepton_p4, p4tr, trigString, nTrig, Form("%s.txt", triggers.at(i).c_str() ) );
                }
            }

        } // end loop on triggers
        if(matchId){
            nTrig = 3;
        }
        else {
            if (match) {
                nTrig = 2;
            } 
            else {
                nTrig = 1;
            }
        }
    }
    pair<int, float> answer;
    answer.first  = nTrig;
    answer.second = dR_min;
    return answer;
}

// struct for trigger matching
struct triggerMatchStruct 
{
    triggerMatchStruct(int NumHLTObjs, float deltaR, int vers, int hltps = -1, int l1ps = -1) 
        : nHLTObjects_(NumHLTObjs)
        , dR_(deltaR)
        , version_(vers)
        , hltps_(hltps)
        , l1ps_(l1ps)
    {}

    int nHLTObjects_;
    float dR_;
    int version_;
    int hltps_;
    int l1ps_;  // not used yet
};

// wrapper around TriggerMatch that takes a TRegExp for matching a class of triggers
triggerMatchStruct MatchTriggerClass(LorentzVector lepton_p4, TPMERegexp* regexp, int pid = 11, double dR_cut = 0.4)
{
    std::pair<int, float> triggerMatchValues = make_pair (0, 99.);
    triggerMatchStruct triggerMatchInfo = triggerMatchStruct(triggerMatchValues.first, triggerMatchValues.second, -1, -1);
    
    unsigned int loopCounts = 0;
    for (unsigned int tidx = 0; tidx < cms2.hlt_trigNames().size(); tidx++) {
        if (regexp->Match(cms2.hlt_trigNames().at(tidx)) == 0)
            continue;

        ++loopCounts;

        // get lepton-trigger matching information
        triggerMatchValues = TriggerMatch(lepton_p4, cms2.hlt_trigNames().at(tidx).Data(), dR_cut, pid);

        int version = -1;
        TString tversion = (*regexp)[1];
        if (tversion.IsDigit())
            version = tversion.Atoi();
        int hltprescale = HLT_prescale(cms2.hlt_trigNames().at(tidx).Data());

        triggerMatchInfo = triggerMatchStruct(triggerMatchValues.first, triggerMatchValues.second, version, hltprescale);
    }

    assert (loopCounts < 2);
    return triggerMatchInfo;
}

//////////////////////////////
// THIS NEEDS TO BE IN CORE //
//////////////////////////////

struct DorkyEventIdentifier
{
    // this is a workaround for not having unique event id's in MC
    unsigned long int run, event,lumi;
    bool operator < (const DorkyEventIdentifier &) const;
    bool operator == (const DorkyEventIdentifier &) const;
};

bool DorkyEventIdentifier::operator < (const DorkyEventIdentifier &other) const
{
    if (run != other.run)
        return run < other.run;
    if (event != other.event)
        return event < other.event;
    if(lumi != other.lumi)
        return lumi < other.lumi;
    return false;
}

bool DorkyEventIdentifier::operator == (const DorkyEventIdentifier &other) const
{
    if (run != other.run)
        return false;
    if (event != other.event)
        return false;
    return true;
}

std::set<DorkyEventIdentifier> already_seen;
bool is_duplicate (const DorkyEventIdentifier &id)
{
    std::pair<std::set<DorkyEventIdentifier>::const_iterator, bool> ret =
        already_seen.insert(id);
    return !ret.second;
}

// transverse mass
float Mt( LorentzVector p4, float met, float met_phi )
{
    return sqrt( 2*met*( p4.pt() - ( p4.Px()*cos(met_phi) + p4.Py()*sin(met_phi) ) ) );
}

#endif // __CINT__

// set good run list
void myBabyMaker::SetGoodRunList(const char* fileName, bool goodRunIsJson)
{
    if (goodRunIsJson)
        set_goodrun_file_json(fileName);
    else
        set_goodrun_file(fileName);

    goodrun_is_json = goodRunIsJson;
}

//------------------------------------------
// Initialize baby ntuple variables
//------------------------------------------
void myBabyMaker::InitBabyNtuple() 
{
    /////////////////////////// 
    // Event Information     //
    ///////////////////////////
    
    // Basic Event Information
    run_    = -1;
    ls_     = -1;
    evt_    = 0;
    weight_ = 1.0;
  
    // Pileup - PUSummaryInfoMaker
    pu_nPUvertices_ = -1;
  
    // Pileup - VertexMaker
    evt_nvtxs_ = -1;
  
    // Pileup - VertexMaker
    evt_ndavtxs_ = -1;

    // event level variables
    nFOels_ = 0;
    nFOmus_ = 0;
    ngsfs_  = 0;
    nmus_   = 0;

    /////////////////////////// 
    // End Event Information //
    ///////////////////////////


    //////////////////////////// 
    // Lepton Information     //
    ////////////////////////////

    id_               = -1;
    pt_               = -999.;
    eta_              = -999.;
    sceta_            = -999.;
    phi_              = -999.;
    scet_             = -999.;
    pfmet_            = -999.;
    pfmetphi_         = -999.;
    hoe_              = -999.;

    lp4_.SetCoordinates(0,0,0,0);     // 4-vector of the lepton
    foel_p4_.SetCoordinates(0,0,0,0); // 4-vector of the highest pt additional FO in the event
    fomu_p4_.SetCoordinates(0,0,0,0); // 4-vector of the highest pt additional FO in the event
    foel_id_   = -999;
    fomu_id_   = -999;
    foel_mass_ = -999.0;
    fomu_mass_ = -999.0;

    iso_                = -999.;
    iso_nps_            = -999.;
    nt_iso_             = -999.;
    nt_iso_nps_         = -999.;
    trck_iso_           = -999.;
    trck_nt_iso_        = -999.;
    ecal_iso_           = -999.;
    ecal_iso_nps_       = -999.;
    ecal_nt_iso_        = -999.;
    ecal_nt_iso_nps_    = -999.;
    hcal_iso_           = -999.;
    hcal_nt_iso_        = -999.;
    nt_pfiso03_         = -999.;
    ch_nt_pfiso03_      = -999.;
    nh_nt_pfiso03_      = -999.;
    em_nt_pfiso03_      = -999.;
    nt_pfiso03_bv_      = -999.;
    ch_nt_pfiso03_bv_   = -999.;
    nh_nt_pfiso03_bv_   = -999.;
    nt_pfiso04_         = -999.;
    ch_nt_pfiso04_      = -999.;
    nh_nt_pfiso04_      = -999.;
    em_nt_pfiso04_      = -999.;
    nt_pfiso04_bv_      = -999.;
    ch_nt_pfiso04_bv_   = -999.;
    nh_nt_pfiso04_bv_   = -999.;
    em_nt_pfiso04_bv_   = -999.;
    nt_radiso_et1p0_    = -999.;
    ch_nt_radiso_et1p0_ = -999.;
    nh_nt_radiso_et1p0_ = -999.;
    em_nt_radiso_et1p0_ = -999.;
    nt_radiso_et0p5_    = -999.;
    ch_nt_radiso_et0p5_ = -999.;
    nh_nt_radiso_et0p5_ = -999.;
    em_nt_radiso_et0p5_ = -999.;

    closestMuon_      = false;
    el_id_smurfV5_    = false;
    el_id_vbtf80_     = false;
    el_id_vbtf90_     = false;
    el_id_effarea_    = -999.0;
    convHitPattern_   = false;
    convPartnerTrack_ = false;
    convMIT_          = false;

    // Z mass variables
    mz_fo_gsf_       = -999.;
    mz_gsf_iso_      = -999.;
    mz_fo_ctf_       = -999.;
    mz_ctf_iso_      = -999.;
    mupsilon_fo_mu_  = -999.;
    mupsilon_mu_iso_ = -999.;

    d0PV_wwV1_       = -999.;
    dzPV_wwV1_       = -999.;

    mu_isCosmic_ = false;;

    mt_                   = -999;
    pfmt_                 = -999;
    q3_                   = false;
    els_exp_innerlayers_  = 999;
    mcid_                 = 0;
    mcmotherid_           = 0;
      
    // HT
    ht_calo_          = -999;           
    ht_calo_L2L3_     = -999;      
    ht_pf_            = -999;            
    ht_pf_L2L3_       = -999;        
    ht_pf_L1FastL2L3_ = -999;

    // MC truth information
    mc3id_         = -999;
    mc3pt_         = -999.;
    mc3dr_         = -999.;
    mc3p4_.SetCoordinates(0,0,0,0);
    leptonIsFromW_ = -999;

    //////////////////////////// 
    // End Lepton Information //
    ////////////////////////////



    //////////////////////////////////////////////////////
    // Fake Rate Numerator & Denominator Selections     //
    //////////////////////////////////////////////////////

    //////////
    // 2012 //
    //////////

    // SS

    // Electrons
    num_el_ssV7_       = false;
    num_el_ssV7_noIso_ = false;
    v1_el_ssV7_        = false;
    v2_el_ssV7_        = false;
    v3_el_ssV7_        = false;
    
    // Muons
    num_mu_ssV5_       = false;        
    num_mu_ssV5_noIso_ = false;  
    fo_mu_ssV5_        = false;         
    fo_mu_ssV5_noIso_  = false;   

    //////////
    // 2011 //
    //////////

    // SS

    // Electrons
    num_el_ssV6_       = false;
    num_el_ssV6_noIso_ = false;
    v1_el_ssV6_        = false;
    v2_el_ssV6_        = false;
    v3_el_ssV6_        = false;

    // Muons
    numNomSSv4_      = false;
    numNomSSv4noIso_ = false; 
    fo_mussV4_04_    = false;
    fo_mussV4_noIso_ = false;

    // WW, HWW

    // Electrons
    num_el_smurfV6_ = false;
    num_el_smurfV6lh_ = false;
    v1_el_smurfV1_  = false;
    v2_el_smurfV1_  = false;
    v3_el_smurfV1_  = false;
    v4_el_smurfV1_  = false;

    // Muons
    num_mu_smurfV6_ = false;
    fo_mu_smurf_04_ = false;
    fo_mu_smurf_10_ = false;


    // OS
    num_el_OSV2_  = false;
    num_mu_OSGV2_ = false;
    num_mu_OSZV2_ = false;
    fo_el_OSV2_   = false;
    fo_mu_OSGV2_  = false;

    // OS
    num_el_OSV3_  = false;
    num_mu_OSGV3_ = false;
    fo_el_OSV3_   = false;
    fo_mu_OSGV3_  = false;
    
    //////////////////////////////////////////////////////
    // End Fake Rate Numerator & Denominator Selections //
    //////////////////////////////////////////////////////

    ///////////////////////  
    // 2012 Triggers     //
    ///////////////////////

    // Electrons
    ele17_CaloIdL_CaloIsoVL_vstar_                          = 0;  
    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_         = 0;  
    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_   = 0;  
    ele8_CaloIdL_CaloIsoVL_vstar_                           = 0;  
    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_          = 0;  
    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_    = 0;  
    ele8_CaloIdT_TrkIdVL_vstar_                             = 0;  
    ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_ = 0;
    ele27_WP80_vstar_ = 0;

    ele17_CaloIdL_CaloIsoVL_version_                        = -1;  
    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version_       = -1;  
    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version_ = -1;  
    ele8_CaloIdL_CaloIsoVL_version_                         = -1;  
    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version_        = -1;  
    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version_  = -1;  
    ele8_CaloIdT_TrkIdVL_version_                           = -1;  
    ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_version_ = -1;
    ele27_WP80_version_ = -1;

    dr_ele17_CaloIdL_CaloIsoVL_vstar_                        = 99.0;  
    dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_       = 99.0;  
    dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_ = 99.0;  
    dr_ele8_CaloIdL_CaloIsoVL_vstar_                         = 99.0;  
    dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_        = 99.0;  
    dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_  = 99.0;  
    dr_ele8_CaloIdT_TrkIdVL_vstar_                           = 99.0;  
    dr_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_ = 99.0;
    dr_ele27_WP80_vstar_ = 99.0;

    hltps_ele17_CaloIdL_CaloIsoVL_vstar_                        = -1;  
    hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_       = -1;  
    hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_ = -1;  
    hltps_ele8_CaloIdL_CaloIsoVL_vstar_                         = -1;  
    hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_        = -1;  
    hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_  = -1;  
    hltps_ele8_CaloIdT_TrkIdVL_vstar_                           = -1;  
    hltps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_ = -1;
    hltps_ele27_WP80_vstar_ = -1;

    l1ps_ele17_CaloIdL_CaloIsoVL_vstar_                        = -1;  
    l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_       = -1;  
    l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_ = -1;  
    l1ps_ele8_CaloIdL_CaloIsoVL_vstar_                         = -1;  
    l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_        = -1;  
    l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_  = -1;  
    l1ps_ele8_CaloIdT_TrkIdVL_vstar_                           = -1;  
    l1ps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_ = -1;
    l1ps_ele27_WP80_vstar_ = -1;

    // Muons
    mu5_vstar_         = 0; 
    mu8_vstar_         = 0;
    mu12_vstar_        = 0; 
    mu17_vstar_        = 0; 
    mu15_eta2p1_vstar_ = 0; 
    mu24_eta2p1_vstar_ = 0; 
    mu30_eta2p1_vstar_ = 0; 
    isomu20_eta2p1_vstar_ = 0; 
    isomu24_eta2p1_vstar_ = 0; 
    isomu30_eta2p1_vstar_ = 0; 

    mu5_version_         = -1; 
    mu8_version_         = -1;
    mu12_version_        = -1; 
    mu17_version_        = -1; 
    mu15_eta2p1_version_ = -1; 
    mu24_eta2p1_version_ = -1; 
    mu30_eta2p1_version_ = -1; 
    isomu20_eta2p1_version_ = 0; 
    isomu24_eta2p1_version_ = 0; 
    isomu30_eta2p1_version_ = 0; 

    dr_mu8_vstar_         = 99.0;
    dr_mu5_vstar_         = 99.0; 
    dr_mu12_vstar_        = 99.0; 
    dr_mu17_vstar_        = 99.0; 
    dr_mu15_eta2p1_vstar_ = 99.0; 
    dr_mu24_eta2p1_vstar_ = 99.0; 
    dr_mu30_eta2p1_vstar_ = 99.0; 
    dr_isomu20_eta2p1_vstar_ = 99.0; 
    dr_isomu24_eta2p1_vstar_ = 99.0; 
    dr_isomu30_eta2p1_vstar_ = 99.0; 

    hltps_mu8_vstar_         = -1;
    hltps_mu5_vstar_         = -1; 
    hltps_mu12_vstar_        = -1; 
    hltps_mu17_vstar_        = -1; 
    hltps_mu15_eta2p1_vstar_ = -1; 
    hltps_mu24_eta2p1_vstar_ = -1; 
    hltps_mu30_eta2p1_vstar_ = -1; 
    hltps_isomu20_eta2p1_vstar_ = -1; 
    hltps_isomu24_eta2p1_vstar_ = -1; 
    hltps_isomu30_eta2p1_vstar_ = -1; 

    l1ps_mu8_vstar_         = -1;
    l1ps_mu5_vstar_         = -1; 
    l1ps_mu12_vstar_        = -1; 
    l1ps_mu17_vstar_        = -1; 
    l1ps_mu15_eta2p1_vstar_ = -1; 
    l1ps_mu24_eta2p1_vstar_ = -1; 
    l1ps_mu30_eta2p1_vstar_ = -1; 
    l1ps_isomu20_eta2p1_vstar_ = -1; 
    l1ps_isomu24_eta2p1_vstar_ = -1; 
    l1ps_isomu30_eta2p1_vstar_ = -1; 


    ///////////////////////  
    // End 2012 Triggers //
    ///////////////////////


   
  
    ///////////////////////  
    // 2011 Triggers     //
    ///////////////////////

    // Electrons
    ele8_vstar_                                             = 0;
    ele8_CaloIdL_TrkIdVL_vstar_                             = 0;
    ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                     = 0;
    ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_          = 0;
    photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_    = 0;

    ele8_version_                                           = -1;
    ele8_CaloIdL_TrkIdVL_version_                           = -1;
    ele8_CaloIdL_CaloIsoVL_Jet40_version_                   = -1;
    ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_version_        = -1;
    photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_version_  = -1;

    dr_ele8_vstar_                                          = 99.0; 
    dr_ele8_CaloIdL_TrkIdVL_vstar_                          = 99.0; 
    dr_ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                  = 99.0; 
    dr_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_       = 99.0;
    dr_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_ = 99.0; 

    hltps_ele8_vstar_                                          = -1; 
    hltps_ele8_CaloIdL_TrkIdVL_vstar_                          = -1; 
    hltps_ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                  = -1; 
    hltps_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_       = -1;
    hltps_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_ = -1; 

    // Muons
    mu3_vstar_          = 0;
    mu15_vstar_         = 0;  
    mu20_vstar_         = 0;  
    mu24_vstar_         = 0;  
    mu30_vstar_         = 0;  
    mu8_Jet40_vstar_    = 0;    

    mu3_version_        = -1;
    mu15_version_       = -1;  
    mu20_version_       = -1;  
    mu24_version_       = -1;  
    mu30_version_       = -1;  
    mu8_Jet40_version_  = -1;    

    dr_mu3_vstar_       = 99.0;
    dr_mu15_vstar_      = 99.0; 
    dr_mu20_vstar_      = 99.0; 
    dr_mu24_vstar_      = 99.0; 
    dr_mu30_vstar_      = 99.0;
    dr_mu8_Jet40_vstar_ = 99.0;

    hltps_mu3_vstar_       = -1;
    hltps_mu15_vstar_      = -1; 
    hltps_mu20_vstar_      = -1; 
    hltps_mu24_vstar_      = -1; 
    hltps_mu30_vstar_      = -1;
    hltps_mu8_Jet40_vstar_ = -1;

    ///////////////////////  
    // End 2011 Triggers //
    ///////////////////////

  
    //////////////
    // Jets     //
    //////////////

    // Calo Jets
    ptj1_       = 0.;
    ptj1_b2b_   = -999.;
    dphij1_b2b_ = -999.;
    nj1_        = 0;

    // PF Jets
    ptpfj1_       = 0.;
    ptpfj1_b2b_   = -999.;
    dphipfj1_b2b_ = -999.;
    npfj1_        = 0;

    // PF L2L3 Corrected jets
    ptpfcj1_        = 0.;
    ptpfcj1_b2b_    = -999.;
    dphipfcj1_b2b_  = -999.;
    npfcj1_         = 0;
    btagpfc_        = false;

    // PF L1FastL2L3 Corrected jets
    emfpfcL1Fj1_       = -999.;
    ptpfcL1Fj1_        = 0.;
    dphipfcL1Fj1_      = -999.;
    ptpfcL1Fj1_b2b_    = -999.;
    dphipfcL1Fj1_b2b_  = -999.;
    npfcL1Fj1_         = 0;
    npfc30L1Fj1_       = 0;
    npfc40L1Fj1_       = 0;
    btagpfcL1F_        = false;

    // btag PF L1FastL2L3 Corrected jets
    ptbtagpfcL1Fj1_        = 0.;
    dphibtagpfcL1Fj1_      = -999.;
    
    //
    nbjet_  = 0;
    dRbNear_ = 99.;
    dRbFar_ = -99.;
    nbpfcjet_  = 0;
    dRbpfcNear_ = 99.;
    dRbpfcFar_ = -99.;

    rho_ = -999.;

    //////////////
    // End Jets //
    //////////////

}

// Book the baby ntuple
void myBabyMaker::MakeBabyNtuple(const char *babyFilename)
{
    babyFile_ = TFile::Open(Form("%s", babyFilename), "RECREATE");
    babyFile_->cd();
    babyTree_ = new TTree("tree", "A Baby Ntuple");
    // babyTree_->SetDirectory(0);

    /////////////////////////// 
    // Event Information     //
    ///////////////////////////

    babyTree_->Branch("run"            , &run_            );
    babyTree_->Branch("ls"             , &ls_             );
    babyTree_->Branch("evt"            , &evt_            );
    babyTree_->Branch("weight"         , &weight_         );
  
    // Pileup
    babyTree_->Branch("pu_nPUvertices" , &pu_nPUvertices_ );
    babyTree_->Branch("evt_nvtxs"      , &evt_nvtxs_      );
    babyTree_->Branch("evt_ndavtxs"    , &evt_ndavtxs_    );

    // event level variables
    babyTree_->Branch("nFOels"         , &nFOels_         );
    babyTree_->Branch("nFOmus"         , &nFOmus_         );
    babyTree_->Branch("ngsfs"          , &ngsfs_          );
    babyTree_->Branch("nmus"           , &nmus_           );

    /////////////////////////// 
    // End Event Information //
    ///////////////////////////
        
        
        
    //////////////////////////// 
    // Lepton Information     //
    ////////////////////////////

    babyTree_->Branch("lp4"                 , &lp4_                 );
    babyTree_->Branch("foel_p4"             , &foel_p4_             );
    babyTree_->Branch("fomu_p4"             , &fomu_p4_             );
    babyTree_->Branch("foel_id"             , &foel_id_             );
    babyTree_->Branch("fomu_id"             , &fomu_id_             );
    babyTree_->Branch("foel_mass"           , &foel_mass_           );
    babyTree_->Branch("fomu_mass"           , &fomu_mass_           );
    babyTree_->Branch("pt"                  , &pt_                  );
    babyTree_->Branch("eta"                 , &eta_                 );
    babyTree_->Branch("sceta"               , &sceta_               );
    babyTree_->Branch("phi"                 , &phi_                 );
    babyTree_->Branch("scet"                , &scet_                );
    babyTree_->Branch("hoe"                 , &hoe_                 );
    babyTree_->Branch("pfmet"               , &pfmet_               );
    babyTree_->Branch("pfmetphi"            , &pfmetphi_            );
    babyTree_->Branch("iso"                 , &iso_                 );
    babyTree_->Branch("iso_nps"             , &iso_nps_             );
    babyTree_->Branch("nt_iso"              , &nt_iso_              );
    babyTree_->Branch("nt_iso_nps"          , &nt_iso_nps_          );
    babyTree_->Branch("trck_iso"            , &trck_iso_            );
    babyTree_->Branch("trck_nt_iso"         , &trck_nt_iso_         );
    babyTree_->Branch("ecal_iso"            , &ecal_iso_            );
    babyTree_->Branch("ecal_iso_nps"        , &ecal_iso_nps_        );
    babyTree_->Branch("ecal_nt_iso"         , &ecal_nt_iso_         );
    babyTree_->Branch("ecal_nt_iso_nps"     , &ecal_nt_iso_nps_     );
    babyTree_->Branch("hcal_iso"            , &hcal_iso_            );
    babyTree_->Branch("hcal_nt_iso"         , &hcal_nt_iso_         );
    babyTree_->Branch("nt_pfiso03"          , &nt_pfiso03_          );
    babyTree_->Branch("ch_nt_pfiso03"       , &ch_nt_pfiso03_       );
    babyTree_->Branch("nh_nt_pfiso03"       , &nh_nt_pfiso03_       );
    babyTree_->Branch("em_nt_pfiso03"       , &em_nt_pfiso03_       );
    babyTree_->Branch("nt_pfiso03_bv"       , &nt_pfiso03_bv_       );
    babyTree_->Branch("ch_nt_pfiso03_bv"    , &ch_nt_pfiso03_bv_    );
    babyTree_->Branch("nh_nt_pfiso03_bv"    , &nh_nt_pfiso03_bv_    );
    babyTree_->Branch("em_nt_pfiso03_bv"    , &em_nt_pfiso03_bv_    );
    babyTree_->Branch("nt_pfiso04"          , &nt_pfiso04_          );
    babyTree_->Branch("ch_nt_pfiso04"       , &ch_nt_pfiso04_       );
    babyTree_->Branch("nh_nt_pfiso04"       , &nh_nt_pfiso04_       );
    babyTree_->Branch("em_nt_pfiso04"       , &em_nt_pfiso04_       );
    babyTree_->Branch("nt_pfiso04_bv"       , &nt_pfiso04_bv_       );
    babyTree_->Branch("ch_nt_pfiso04_bv"    , &ch_nt_pfiso04_bv_    );
    babyTree_->Branch("nh_nt_pfiso04_bv"    , &nh_nt_pfiso04_bv_    );
    babyTree_->Branch("em_nt_pfiso04_bv"    , &em_nt_pfiso04_bv_    );
    babyTree_->Branch("nt_radiso_et1p0"     , &nt_radiso_et1p0_     );
    babyTree_->Branch("ch_nt_radiso_et1p0"  , &ch_nt_radiso_et1p0_  );
    babyTree_->Branch("nh_nt_radiso_et1p0"  , &nh_nt_radiso_et1p0_  );
    babyTree_->Branch("em_nt_radiso_et1p0"  , &em_nt_radiso_et1p0_  );
    babyTree_->Branch("nt_radiso_et0p5"     , &nt_radiso_et0p5_     );
    babyTree_->Branch("ch_nt_radiso_et0p5"  , &ch_nt_radiso_et0p5_  );
    babyTree_->Branch("nh_nt_radiso_et0p5"  , &nh_nt_radiso_et0p5_  );
    babyTree_->Branch("em_nt_radiso_et0p5"  , &em_nt_radiso_et0p5_  );
    babyTree_->Branch("id"                  , &id_                  );
    babyTree_->Branch("closestMuon"         , &closestMuon_         );
    babyTree_->Branch("el_id_smurfV5"       , &el_id_smurfV5_       );
    babyTree_->Branch("el_id_vbtf80"        , &el_id_vbtf80_        );
    babyTree_->Branch("el_id_vbtf90"        , &el_id_vbtf90_        );
    babyTree_->Branch("el_id_effarea"       , &el_id_effarea_       );
    babyTree_->Branch("conv0MissHits"       , &conv0MissHits_       );
    babyTree_->Branch("convHitPattern"      , &convHitPattern_      );
    babyTree_->Branch("convPartnerTrack"    , &convPartnerTrack_    );
    babyTree_->Branch("convMIT"             , &convMIT_             );
    babyTree_->Branch("mt"                  , &mt_                  );
    babyTree_->Branch("pfmt"                , &pfmt_                );
    babyTree_->Branch("q3"                  , &q3_                  );
    babyTree_->Branch("els_exp_innerlayers" , &els_exp_innerlayers_ );
    babyTree_->Branch("d0PV_wwV1"           , &d0PV_wwV1_           );
    babyTree_->Branch("dzPV_wwV1"           , &dzPV_wwV1_           );
    babyTree_->Branch("ht_calo"             , &ht_calo_             );
    babyTree_->Branch("ht_calo_L2L3"        , &ht_calo_L2L3_        );
    babyTree_->Branch("ht_pf"               , &ht_pf_               );
    babyTree_->Branch("ht_pf_L2L3"          , &ht_pf_L2L3_          );
    babyTree_->Branch("ht_pf_L1FastL2L3"    , &ht_pf_L1FastL2L3_    );
    babyTree_->Branch("mcid"                , &mcid_                );
    babyTree_->Branch("mcmotherid"          , &mcmotherid_          );
    babyTree_->Branch("mc3id"               , &mc3id_               );
    babyTree_->Branch("mc3pt"               , &mc3pt_               );
    babyTree_->Branch("mc3p4"               , &mc3p4_               );
    babyTree_->Branch("mc3dr"               , &mc3dr_               );
    babyTree_->Branch("leptonIsFromW"       , &leptonIsFromW_       );
    babyTree_->Branch("mu_isCosmic"         , &mu_isCosmic_         );

    // Z mass variables
    babyTree_->Branch("mz_fo_gsf"      , &mz_fo_gsf_      );
    babyTree_->Branch("mz_gsf_iso"     , &mz_gsf_iso_     );
    babyTree_->Branch("mz_fo_ctf"      , &mz_fo_ctf_      );
    babyTree_->Branch("mz_ctf_iso"     , &mz_ctf_iso_     );
    babyTree_->Branch("mupsilon_fo_mu" , &mupsilon_fo_mu_ );
    babyTree_->Branch("mupsilon_mu_iso", &mupsilon_mu_iso_);

    //////////////////////////// 
    // End Lepton Information //
    ////////////////////////////

    //////////////////////////////////////////////////////
    // Fake Rate Numerator & Denominator Selections     //
    //////////////////////////////////////////////////////

    //////////
    // 2012 //
    //////////

    // SS
    // Electrons
    babyTree_->Branch("num_el_ssV7"       , &num_el_ssV7_       );
    babyTree_->Branch("num_el_ssV7_noIso" , &num_el_ssV7_noIso_ );
    babyTree_->Branch("v1_el_ssV7"        , &v1_el_ssV7_        );
    babyTree_->Branch("v2_el_ssV7"        , &v2_el_ssV7_        );
    babyTree_->Branch("v3_el_ssV7"        , &v3_el_ssV7_        );
    
    // Muons
    babyTree_->Branch("num_mu_ssV5"       , &num_mu_ssV5_       );
    babyTree_->Branch("num_mu_ssV5_noIso" , &num_mu_ssV5_noIso_ );
    babyTree_->Branch("fo_mu_ssV5"        , &fo_mu_ssV5_        );
    babyTree_->Branch("fo_mu_ssV5_noIso"  , &fo_mu_ssV5_noIso_  );


    //////////
    // 2011 //
    //////////

    // SS

    // Electrons
    babyTree_->Branch("num_el_ssV6"       , &num_el_ssV6_       );
    babyTree_->Branch("num_el_ssV6_noIso" , &num_el_ssV6_noIso_ );
    babyTree_->Branch("v1_el_ssV6"        , &v1_el_ssV6_        );
    babyTree_->Branch("v2_el_ssV6"        , &v2_el_ssV6_        );
    babyTree_->Branch("v3_el_ssV6"        , &v3_el_ssV6_        );

    // Muons
    babyTree_->Branch("numNomSSv4"      , &numNomSSv4_      );
    babyTree_->Branch("numNomSSv4noIso" , &numNomSSv4noIso_ );
    babyTree_->Branch("fo_mussV4_04"    , &fo_mussV4_04_    );
    babyTree_->Branch("fo_mussV4_noIso" , &fo_mussV4_noIso_ );

    // WW, HWW

    // Electrons
    babyTree_->Branch("num_el_smurfV6"   , &num_el_smurfV6_   );
    babyTree_->Branch("num_el_smurfV6lh" , &num_el_smurfV6lh_ );
    babyTree_->Branch("v1_el_smurfV1"    , &v1_el_smurfV1_    );
    babyTree_->Branch("v2_el_smurfV1"    , &v2_el_smurfV1_    );
    babyTree_->Branch("v3_el_smurfV1"    , &v3_el_smurfV1_    );
    babyTree_->Branch("v4_el_smurfV1"    , &v4_el_smurfV1_    );

    // Muons
    babyTree_->Branch("num_mu_smurfV6",  &num_mu_smurfV6_ );
    babyTree_->Branch("fo_mu_smurf_04",  &fo_mu_smurf_04_ );
    babyTree_->Branch("fo_mu_smurf_10",  &fo_mu_smurf_10_ );
  
    // OS
    babyTree_->Branch("num_el_OSV2"  , &num_el_OSV2_  );
    babyTree_->Branch("num_mu_OSGV2" , &num_mu_OSGV2_ );
    babyTree_->Branch("num_mu_OSZV2" , &num_mu_OSZV2_ );
    babyTree_->Branch("fo_el_OSV2"   , &fo_el_OSV2_   );
    babyTree_->Branch("fo_mu_OSGV2"  , &fo_mu_OSGV2_  );

    babyTree_->Branch("num_el_OSV3"  , &num_el_OSV3_  );
    babyTree_->Branch("num_mu_OSGV3" , &num_mu_OSGV3_ );
    babyTree_->Branch("fo_el_OSV3"   , &fo_el_OSV3_   );
    babyTree_->Branch("fo_mu_OSGV3"  , &fo_mu_OSGV3_  );


    //////////////////////////////////////////////////////
    // End Fake Rate Numerator & Denominator Selections //
    //////////////////////////////////////////////////////

    ///////////////////////  
    // Triggers          //
    ///////////////////////

    // Electrons
    babyTree_->Branch("ele8_vstar"                                                        , &ele8_vstar_                                                        ); 
    babyTree_->Branch("ele8_CaloIdL_TrkIdVL_vstar"                                        , &ele8_CaloIdL_TrkIdVL_vstar_                                        ); 
    babyTree_->Branch("ele8_CaloIdL_CaloIsoVL_Jet40_vstar"                                , &ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                                ); 
    babyTree_->Branch("ele8_CaloIdL_CaloIsoVL_vstar"                                      , &ele8_CaloIdL_CaloIsoVL_vstar_                                      ); 
    babyTree_->Branch("ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar"                     , &ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_                     ); 
    babyTree_->Branch("ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"                     , &ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                     ); 
    babyTree_->Branch("ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"               , &ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_               ); 
    babyTree_->Branch("ele8_CaloIdT_TrkIdVL_vstar"                                        , &ele8_CaloIdT_TrkIdVL_vstar_                                        ); 
    babyTree_->Branch("ele17_CaloIdL_CaloIsoVL_vstar"                                     , &ele17_CaloIdL_CaloIsoVL_vstar_                                     ); 
    babyTree_->Branch("ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"                    , &ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                    ); 
    babyTree_->Branch("ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"              , &ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_              ); 
    babyTree_->Branch("photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar"               , &photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_               ); 
    babyTree_->Branch("ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar"       , &ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_       ); 
    babyTree_->Branch("ele27_WP80_vstar"                                                  , &ele27_WP80_vstar_                                                  ); 

    babyTree_->Branch("ele8_version"                                                      , &ele8_version_                                                      ); 
    babyTree_->Branch("ele8_CaloIdL_TrkIdVL_version"                                      , &ele8_CaloIdL_TrkIdVL_version_                                      ); 
    babyTree_->Branch("ele8_CaloIdL_CaloIsoVL_Jet40_version"                              , &ele8_CaloIdL_CaloIsoVL_Jet40_version_                              ); 
    babyTree_->Branch("ele8_CaloIdL_CaloIsoVL_version"                                    , &ele8_CaloIdL_CaloIsoVL_version_                                    ); 
    babyTree_->Branch("ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_version"                   , &ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_version_                   ); 
    babyTree_->Branch("ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version"                   , &ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version_                   ); 
    babyTree_->Branch("ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version"             , &ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version_             ); 
    babyTree_->Branch("ele8_CaloIdT_TrkIdVL_version"                                      , &ele8_CaloIdT_TrkIdVL_version_                                      ); 
    babyTree_->Branch("ele17_CaloIdL_CaloIsoVL_version"                                   , &ele17_CaloIdL_CaloIsoVL_version_                                   ); 
    babyTree_->Branch("ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version"                  , &ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version_                  ); 
    babyTree_->Branch("ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version"            , &ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version_            ); 
    babyTree_->Branch("photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_version"             , &photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_version_             ); 
    babyTree_->Branch("ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_version"     , &ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_version_     ); 
    babyTree_->Branch("ele27_WP80_version"                                                , &ele27_WP80_version_                                                ); 

    babyTree_->Branch("dr_ele8_vstar"                                                     , &dr_ele8_vstar_                                                     ); 
    babyTree_->Branch("dr_ele8_CaloIdL_TrkIdVL_vstar"                                     , &dr_ele8_CaloIdL_TrkIdVL_vstar_                                     ); 
    babyTree_->Branch("dr_ele8_CaloIdL_CaloIsoVL_Jet40_vstar"                             , &dr_ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                             ); 
    babyTree_->Branch("dr_ele8_CaloIdL_CaloIsoVL_vstar"                                   , &dr_ele8_CaloIdL_CaloIsoVL_vstar_                                   ); 
    babyTree_->Branch("dr_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar"                  , &dr_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_                  ); 
    babyTree_->Branch("dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"                  , &dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                  ); 
    babyTree_->Branch("dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"            , &dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_            ); 
    babyTree_->Branch("dr_ele8_CaloIdT_TrkIdVL_vstar"                                     , &dr_ele8_CaloIdT_TrkIdVL_vstar_                                     ); 
    babyTree_->Branch("dr_ele17_CaloIdL_CaloIsoVL_vstar"                                  , &dr_ele17_CaloIdL_CaloIsoVL_vstar_                                  ); 
    babyTree_->Branch("dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"                 , &dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                 ); 
    babyTree_->Branch("dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"           , &dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_           ); 
    babyTree_->Branch("dr_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar"            , &dr_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_            ); 
    babyTree_->Branch("dr_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar"    , &dr_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_    ); 
    babyTree_->Branch("dr_ele27_WP80_vstar"                                               , &dr_ele27_WP80_vstar_                                               ); 

    babyTree_->Branch("hltps_ele8_vstar"                                                  , &hltps_ele8_vstar_                                                  ); 
    babyTree_->Branch("hltps_ele8_CaloIdL_TrkIdVL_vstar"                                  , &hltps_ele8_CaloIdL_TrkIdVL_vstar_                                  ); 
    babyTree_->Branch("hltps_ele8_CaloIdL_CaloIsoVL_Jet40_vstar"                          , &hltps_ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                          ); 
    babyTree_->Branch("hltps_ele8_CaloIdL_CaloIsoVL_vstar"                                , &hltps_ele8_CaloIdL_CaloIsoVL_vstar_                                ); 
    babyTree_->Branch("hltps_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar"               , &hltps_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_               ); 
    babyTree_->Branch("hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"               , &hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_               ); 
    babyTree_->Branch("hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"         , &hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_         ); 
    babyTree_->Branch("hltps_ele8_CaloIdT_TrkIdVL_vstar"                                  , &hltps_ele8_CaloIdT_TrkIdVL_vstar_                                  ); 
    babyTree_->Branch("hltps_ele17_CaloIdL_CaloIsoVL_vstar"                               , &hltps_ele17_CaloIdL_CaloIsoVL_vstar_                               ); 
    babyTree_->Branch("hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"              , &hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_              ); 
    babyTree_->Branch("hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"        , &hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_        ); 
    babyTree_->Branch("hltps_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar"         , &hltps_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_         ); 
    babyTree_->Branch("hltps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar" , &hltps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_ ); 
    babyTree_->Branch("hltps_ele27_WP80_vstar"                                            , &hltps_ele27_WP80_vstar_                                            ); 

    babyTree_->Branch("l1ps_ele8_CaloIdL_CaloIsoVL_vstar"                                 , &l1ps_ele8_CaloIdL_CaloIsoVL_vstar_                                ); 
    babyTree_->Branch("l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"                , &l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_               ); 
    babyTree_->Branch("l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"          , &l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_         ); 
    babyTree_->Branch("l1ps_ele8_CaloIdT_TrkIdVL_vstar"                                   , &l1ps_ele8_CaloIdT_TrkIdVL_vstar_                                  ); 
    babyTree_->Branch("l1ps_ele17_CaloIdL_CaloIsoVL_vstar"                                , &l1ps_ele17_CaloIdL_CaloIsoVL_vstar_                               ); 
    babyTree_->Branch("l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar"               , &l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_              ); 
    babyTree_->Branch("l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar"         , &l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_        ); 
    babyTree_->Branch("l1ps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar"  , &l1ps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_ ); 
    babyTree_->Branch("l1ps_ele27_WP80_vstar"                                             , &l1ps_ele27_WP80_vstar_                                            ); 

    babyTree_->Branch("mu3_vstar"                                                         , &mu3_vstar_                                                         ); 
    babyTree_->Branch("mu5_vstar"                                                         , &mu5_vstar_                                                         ); 
    babyTree_->Branch("mu8_vstar"                                                         , &mu8_vstar_                                                         ); 
    babyTree_->Branch("mu12_vstar"                                                        , &mu12_vstar_                                                        ); 
    babyTree_->Branch("mu15_vstar"                                                        , &mu15_vstar_                                                        ); 
    babyTree_->Branch("mu17_vstar"                                                        , &mu17_vstar_                                                        ); 
    babyTree_->Branch("mu20_vstar"                                                        , &mu20_vstar_                                                        ); 
    babyTree_->Branch("mu24_vstar"                                                        , &mu24_vstar_                                                        ); 
    babyTree_->Branch("mu30_vstar"                                                        , &mu30_vstar_                                                        ); 
    babyTree_->Branch("mu15_eta2p1_vstar"                                                 , &mu15_eta2p1_vstar_                                                 ); 
    babyTree_->Branch("mu24_eta2p1_vstar"                                                 , &mu24_eta2p1_vstar_                                                 ); 
    babyTree_->Branch("mu30_eta2p1_vstar"                                                 , &mu30_eta2p1_vstar_                                                 ); 
    babyTree_->Branch("mu8_Jet40_vstar"                                                   , &mu8_Jet40_vstar_                                                   ); 
    babyTree_->Branch("isomu20_eta2p1_vstar"                                              , &isomu20_eta2p1_vstar_                                              ); 
    babyTree_->Branch("isomu24_eta2p1_vstar"                                              , &isomu24_eta2p1_vstar_                                              ); 
    babyTree_->Branch("isomu30_eta2p1_vstar"                                              , &isomu30_eta2p1_vstar_                                              ); 

    babyTree_->Branch("mu3_version"                                                       , &mu3_version_                                                       ); 
    babyTree_->Branch("mu5_version"                                                       , &mu5_version_                                                       ); 
    babyTree_->Branch("mu8_version"                                                       , &mu8_version_                                                       ); 
    babyTree_->Branch("mu12_version"                                                      , &mu12_version_                                                      ); 
    babyTree_->Branch("mu15_version"                                                      , &mu15_version_                                                      ); 
    babyTree_->Branch("mu17_version"                                                      , &mu17_version_                                                      ); 
    babyTree_->Branch("mu20_version"                                                      , &mu20_version_                                                      ); 
    babyTree_->Branch("mu24_version"                                                      , &mu24_version_                                                      ); 
    babyTree_->Branch("mu30_version"                                                      , &mu30_version_                                                      ); 
    babyTree_->Branch("mu15_eta2p1_version"                                               , &mu15_eta2p1_version_                                               ); 
    babyTree_->Branch("mu24_eta2p1_version"                                               , &mu24_eta2p1_version_                                               ); 
    babyTree_->Branch("mu30_eta2p1_version"                                               , &mu30_eta2p1_version_                                               ); 
    babyTree_->Branch("mu8_Jet40_version"                                                 , &mu8_Jet40_version_                                                 ); 
    babyTree_->Branch("isomu20_eta2p1_version"                                            , &isomu20_eta2p1_version_                                            ); 
    babyTree_->Branch("isomu24_eta2p1_version"                                            , &isomu24_eta2p1_version_                                            ); 
    babyTree_->Branch("isomu30_eta2p1_version"                                            , &isomu30_eta2p1_version_                                            ); 

    babyTree_->Branch("dr_mu3_vstar"                                                      , &dr_mu3_vstar_                                                      ); 
    babyTree_->Branch("dr_mu5_vstar"                                                      , &dr_mu5_vstar_                                                      ); 
    babyTree_->Branch("dr_mu8_vstar"                                                      , &dr_mu8_vstar_                                                      ); 
    babyTree_->Branch("dr_mu12_vstar"                                                     , &dr_mu12_vstar_                                                     ); 
    babyTree_->Branch("dr_mu15_vstar"                                                     , &dr_mu15_vstar_                                                     ); 
    babyTree_->Branch("dr_mu17_vstar"                                                     , &dr_mu17_vstar_                                                     ); 
    babyTree_->Branch("dr_mu20_vstar"                                                     , &dr_mu20_vstar_                                                     ); 
    babyTree_->Branch("dr_mu24_vstar"                                                     , &dr_mu24_vstar_                                                     ); 
    babyTree_->Branch("dr_mu30_vstar"                                                     , &dr_mu30_vstar_                                                     ); 
    babyTree_->Branch("dr_mu15_eta2p1_vstar"                                              , &dr_mu15_eta2p1_vstar_                                              ); 
    babyTree_->Branch("dr_mu24_eta2p1_vstar"                                              , &dr_mu24_eta2p1_vstar_                                              ); 
    babyTree_->Branch("dr_mu30_eta2p1_vstar"                                              , &dr_mu30_eta2p1_vstar_                                              ); 
    babyTree_->Branch("dr_mu8_Jet40_vstar"                                                , &dr_mu8_Jet40_vstar_                                                ); 
    babyTree_->Branch("dr_isomu20_eta2p1_vstar"                                           , &dr_isomu20_eta2p1_vstar_                                           ); 
    babyTree_->Branch("dr_isomu24_eta2p1_vstar"                                           , &dr_isomu24_eta2p1_vstar_                                           ); 
    babyTree_->Branch("dr_isomu30_eta2p1_vstar"                                           , &dr_isomu30_eta2p1_vstar_                                           ); 

    babyTree_->Branch("hltps_mu3_vstar"                                                   , &hltps_mu3_vstar_                                                   ); 
    babyTree_->Branch("hltps_mu5_vstar"                                                   , &hltps_mu5_vstar_                                                   ); 
    babyTree_->Branch("hltps_mu8_vstar"                                                   , &hltps_mu8_vstar_                                                   ); 
    babyTree_->Branch("hltps_mu12_vstar"                                                  , &hltps_mu12_vstar_                                                  ); 
    babyTree_->Branch("hltps_mu15_vstar"                                                  , &hltps_mu15_vstar_                                                  ); 
    babyTree_->Branch("hltps_mu17_vstar"                                                  , &hltps_mu17_vstar_                                                  ); 
    babyTree_->Branch("hltps_mu20_vstar"                                                  , &hltps_mu20_vstar_                                                  ); 
    babyTree_->Branch("hltps_mu24_vstar"                                                  , &hltps_mu24_vstar_                                                  ); 
    babyTree_->Branch("hltps_mu30_vstar"                                                  , &hltps_mu30_vstar_                                                  ); 
    babyTree_->Branch("hltps_mu15_eta2p1_vstar"                                           , &hltps_mu15_eta2p1_vstar_                                           ); 
    babyTree_->Branch("hltps_mu24_eta2p1_vstar"                                           , &hltps_mu24_eta2p1_vstar_                                           ); 
    babyTree_->Branch("hltps_mu30_eta2p1_vstar"                                           , &hltps_mu30_eta2p1_vstar_                                           ); 
    babyTree_->Branch("hltps_mu8_Jet40_vstar"                                             , &hltps_mu8_Jet40_vstar_                                             ); 
    babyTree_->Branch("hltps_isomu20_eta2p1_vstar"                                        , &hltps_isomu20_eta2p1_vstar_                                        ); 
    babyTree_->Branch("hltps_isomu24_eta2p1_vstar"                                        , &hltps_isomu24_eta2p1_vstar_                                        ); 
    babyTree_->Branch("hltps_isomu30_eta2p1_vstar"                                        , &hltps_isomu30_eta2p1_vstar_                                        );

    babyTree_->Branch("l1ps_mu5_vstar"                                                    , &l1ps_mu5_vstar_                                                    );
    babyTree_->Branch("l1ps_mu8_vstar"                                                    , &l1ps_mu8_vstar_                                                    );
    babyTree_->Branch("l1ps_mu12_vstar"                                                   , &l1ps_mu12_vstar_                                                   );
    babyTree_->Branch("l1ps_mu17_vstar"                                                   , &l1ps_mu17_vstar_                                                   );
    babyTree_->Branch("l1ps_mu15_eta2p1_vstar"                                            , &l1ps_mu15_eta2p1_vstar_                                            );
    babyTree_->Branch("l1ps_mu24_eta2p1_vstar"                                            , &l1ps_mu24_eta2p1_vstar_                                            );
    babyTree_->Branch("l1ps_mu30_eta2p1_vstar"                                            , &l1ps_mu30_eta2p1_vstar_                                            );
    babyTree_->Branch("l1ps_isomu20_eta2p1_vstar"                                         , &l1ps_isomu20_eta2p1_vstar_                                         );
    babyTree_->Branch("l1ps_isomu24_eta2p1_vstar"                                         , &l1ps_isomu24_eta2p1_vstar_                                         );
    babyTree_->Branch("l1ps_isomu30_eta2p1_vstar"                                         , &l1ps_isomu30_eta2p1_vstar_                                         );

    ///////////////////////  
    // End 2011 Triggers //
    ///////////////////////
        
    //////////////
    // Jets     //
    //////////////

    // Information to do offline jet trigger selection
    babyTree_->Branch("ptj1"         , &ptj1_         );
    babyTree_->Branch("nj1"          , &nj1_          );
    babyTree_->Branch("ptj1_b2b"     , &ptj1_b2b_     );
    babyTree_->Branch("dphij1_b2b"   , &dphij1_b2b_   );
    babyTree_->Branch("ptpfj1"       , &ptpfj1_       );
    babyTree_->Branch("npfj1"        , &npfj1_        );
    babyTree_->Branch("ptpfj1_b2b"   , &ptpfj1_b2b_   );
    babyTree_->Branch("dphipfj1_b2b" , &dphipfj1_b2b_ );
      
    // PF L2L3 Corrected jets
    babyTree_->Branch("ptpfcj1"      , &ptpfcj1_      );
    babyTree_->Branch("npfcj1"       , &npfcj1_       );
    babyTree_->Branch("ptpfcj1_b2b"  , &ptpfcj1_b2b_  );
    babyTree_->Branch("dphipfcj1_b2b", &dphipfcj1_b2b_);
    babyTree_->Branch("btagpfc"      , &btagpfc_      );
      
    // PF L1FastL2L3 Corrected jets         
    babyTree_->Branch("emfpfcL1Fj1"     , &emfpfcL1Fj1_      );
    babyTree_->Branch("ptpfcL1Fj1"      , &ptpfcL1Fj1_       );       
    babyTree_->Branch("dphipfcL1Fj1"    , &dphipfcL1Fj1_     );       
    babyTree_->Branch("npfcL1Fj1"       , &npfcL1Fj1_        );
    babyTree_->Branch("npfc30L1Fj1"     , &npfc30L1Fj1_      );
    babyTree_->Branch("npfc40L1Fj1"     , &npfc40L1Fj1_      );
    babyTree_->Branch("ptpfcL1Fj1_b2b"  , &ptpfcL1Fj1_b2b_   );       
    babyTree_->Branch("dphipfcL1Fj1_b2b", &dphipfcL1Fj1_b2b_ );     
    babyTree_->Branch("btagpfcL1F"      , &btagpfcL1F_       );

    // B-tagged PF L1FastL2L3 Corrected jets         
    babyTree_->Branch("ptbtagpfcL1Fj1"   , &ptbtagpfcL1Fj1_   );
    babyTree_->Branch("dphibtagpfcL1Fj1" , &dphibtagpfcL1Fj1_ );
    
    // B Tagging
    babyTree_->Branch("nbjet"        , &nbjet_        );
    babyTree_->Branch("dRNear"       , &dRbNear_      );
    babyTree_->Branch("dRFar"        , &dRbFar_       );
      
    babyTree_->Branch("nbpfcjet"     , &nbpfcjet_     );
    babyTree_->Branch("dRpfcNear"    , &dRbpfcNear_   );
    babyTree_->Branch("dRpfcFar"     , &dRbpfcFar_    );

    babyTree_->Branch("rho", &rho_);
    
    //////////////
    // End Jets //
    //////////////
}

// Fill the baby
void myBabyMaker::FillBabyNtuple()
{ 
    babyTree_->Fill(); 
}

// Close the baby
void myBabyMaker::CloseBabyNtuple()
{
    babyFile_->cd();
    babyTree_->Write();
    babyFile_->Close();
}

// constructor
myBabyMaker::myBabyMaker () 
    : nEvents_(-1)
    , verbose_(false)
    , ele8_regexp                                                 ("HLT_Ele8_v(\\d+)"                                                 , "o")
    , ele8_CaloIdL_TrkIdVL_regexp                                 ("HLT_Ele8_CaloIdL_TrkIdVL_v(\\d+)"                                 , "o")
    , ele8_CaloIdL_CaloIsoVL_regexp                               ("HLT_Ele8_CaloIdL_CaloIsoVL_v(\\d+)"                               , "o")
    , ele8_CaloIdL_CaloIsoVL_Jet40_regexp                         ("HLT_Ele8_CaloIdL_CaloIsoVL_Jet40_v(\\d+)"                         , "o")
    , ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_regexp              ("HLT_Ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_v(\\d+)"              , "o")
    , ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_regexp              ("HLT_Ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_v(\\d+)"              , "o")
    , ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_regexp        ("HLT_Ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_v(\\d+)"        , "o")
    , ele8_CaloIdT_TrkIdVL_regexp                                 ("HLT_Ele8_CaloIdT_TrkIdVL_v(\\d+)"                                 , "o")
    , ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_regexp        ("HLT_Ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_v(\\d+)"              , "o")
    , ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_regexp        ("HLT_Ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_v(\\d+)"              , "o")
    , ele17_CaloIdL_CaloIsoVL_regexp                              ("HLT_Ele17_CaloIdL_CaloIsoVL_v(\\d+)"                              , "o")
    , ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_regexp             ("HLT_Ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_v(\\d+)"             , "o")
    , ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_regexp       ("HLT_Ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_v(\\d+)"       , "o")
    , ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_rexexp("HLT_Ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_v(\\d+)", "o")
    , ele27_WP80_rexexp                                           ("HLT_Ele27_WP80_v(\\d+)"                                           , "o")
    , photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_regexp        ("HLT_Photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_v(\\d+)"        , "o")
    , mu3_regexp           ("HLT_Mu3_v(\\d+)"           , "o")
    , mu5_regexp           ("HLT_Mu5_v(\\d+)"           , "o")          
    , mu8_regexp           ("HLT_Mu8_v(\\d+)"           , "o")      
    , mu12_regexp          ("HLT_Mu12_v(\\d+)"          , "o")     
    , mu15_regexp          ("HLT_Mu15_v(\\d+)"          , "o")     
    , mu17_regexp          ("HLT_Mu17_v(\\d+)"          , "o")     
    , mu20_regexp          ("HLT_Mu20_v(\\d+)"          , "o")     
    , mu24_regexp          ("HLT_Mu24_v(\\d+)"          , "o")     
    , mu30_regexp          ("HLT_Mu30_v(\\d+)"          , "o")     
    , mu15_eta2p1_regexp   ("HLT_Mu15_eta2p1_v(\\d+)"   , "o")     
    , mu24_eta2p1_regexp   ("HLT_Mu24_eta2p1_v(\\d+)"   , "o")     
    , mu30_eta2p1_regexp   ("HLT_Mu30_eta2p1_v(\\d+)"   , "o")     
    , mu8_Jet40_regexp     ("HLT_Mu8_Jet40_v(\\d+)"     , "o")
    , isomu20_eta2p1_regexp("HLT_IsoMu20_eta2p1_v(\\d+)", "o")
    , isomu24_eta2p1_regexp("HLT_IsoMu24_eta2p1_v(\\d+)", "o")
    , isomu30_eta2p1_regexp("HLT_IsoMu30_eta2p1_v(\\d+)", "o")
{
}

//-----------------------------------
// Looper code starts here
// eormu=-1 do both e and mu
//      =11 do electrons
//      =13 do muons
//-----------------------------------
void myBabyMaker::ScanChain(TChain* chain, const char *babyFilename, bool isData, int eormu, bool applyFOfilter, const std::string& jetcorrPath)
{

    already_seen.clear();

    // Make a baby ntuple
    MakeBabyNtuple(babyFilename);

    // Jet Corrections
    std::vector<std::string> jetcorr_pf_L2L3_filenames;
    if (isData) {        
        string data_pf_l2 = jetcorrPath;
        data_pf_l2.append("/GR_R_42_V14_AK5PF_L2Relative.txt");
        string data_pf_l3 = jetcorrPath;
        data_pf_l3.append("/GR_R_42_V14_AK5PF_L3Absolute.txt");
        jetcorr_pf_L2L3_filenames.push_back(data_pf_l2.c_str());
        jetcorr_pf_L2L3_filenames.push_back(data_pf_l3.c_str());
    }
    else {
        string mc_pf_l2 = jetcorrPath;
        mc_pf_l2.append("/START41_V0_AK5PF_L2Relative.txt");
        string mc_pf_l3 = jetcorrPath;
        mc_pf_l3.append("/START41_V0_AK5PF_L3Absolute.txt");
        jetcorr_pf_L2L3_filenames.push_back(mc_pf_l2.c_str());
        jetcorr_pf_L2L3_filenames.push_back(mc_pf_l3.c_str());
    }    

    std::cout << "making jet corrector with the following files: " << std::endl;
    for (unsigned int idx = 0; idx < jetcorr_pf_L2L3_filenames.size(); idx++)
        std::cout << jetcorr_pf_L2L3_filenames.at(idx) << std::endl;
    FactorizedJetCorrector *jet_pf_L2L3corrector = makeJetCorrector(jetcorr_pf_L2L3_filenames);

    // The deltaR requirement between objects and jets to remove the jet trigger dependence
    float deltaRCut   = 1.0;
    float deltaPhiCut = 2.5;

    //--------------------------
    // File and Event Loop
    //---------------------------
    
    // benchmark
    TBenchmark bmark;
    bmark.Start("benchmark");
    
    int i_permilleOld = 0;
    unsigned int nEventsTotal = 0;
    unsigned int nEventsChain = 0;
    int nEvents = nEvents_; 
    if (nEvents==-1){
        nEventsChain = chain->GetEntries();
    } else {
        nEventsChain = nEvents;
    }
    TObjArray *listOfFiles = chain->GetListOfFiles();
    TIter fileIter(listOfFiles);
    bool finish_looping = false;

    std::cout << "looping on " << nEventsChain << " out of " << chain->GetEntries() << " events..." << std::endl;

    while(TChainElement *currentFile = (TChainElement*)fileIter.Next())
    {
        if (finish_looping) {
            break;
        }

        TString filename = currentFile->GetTitle();
        if (verbose_)
        {
            cout << filename << endl;
        }
    
        TFile* f = TFile::Open(filename.Data());
        TTree *tree = (TTree*)f->Get("Events");
        cms2.Init(tree);

        unsigned int nEntries = tree->GetEntries();
        unsigned int nGoodEvents(0);
        unsigned int nLoop = nEntries;
        unsigned int z;

        // Event Loop
        for( z = 0; z < nLoop; z++)
        { 
            cms2.GetEntry(z);

            if (nEventsTotal >= nEventsChain) {
                finish_looping = true;
                break;
            }

            if(isData){
                // Good  Runs
                if (goodrun_is_json) {
                    if(!goodrun_json(evt_run(), evt_lumiBlock())) continue;   
                }
                else {
                    if(!goodrun(evt_run(), evt_lumiBlock())) continue;   
                }

                // check for duplicated
                DorkyEventIdentifier id = {evt_run(), evt_event(), evt_lumiBlock()};
                if (is_duplicate(id) ) { 
                    cout << "\t! ERROR: found duplicate." << endl;
                    continue;
                }
            }

            // looper progress
            ++nEventsTotal;
            ++nGoodEvents;
             int i_permille = (int)floor(1000 * nEventsTotal / float(nEventsChain));
             if (i_permille != i_permilleOld) {
                 printf("  \015\033[32m ---> \033[1m\033[31m%4.1f%%" "\033[0m\033[32m <---\033[0m\015", i_permille/10.);
                 fflush(stdout);
                 i_permilleOld = i_permille;
             }
      
            // Event cleaning (careful, it requires technical bits)
            //if (!cleaning_BPTX(isData))   continue;
            //if (!cleaning_beamHalo())   continue;
            //if (!cleaning_goodVertexAugust2010()) continue;
            //if (!cleaning_goodTracks()) continue;
            if (!cleaning_standardApril2011()) continue;

	  		// Loop over jets and see what is btagged
            // Medium operating point from https://twiki.cern.ch/twiki/bin/view/CMS/BTagPerformanceOP
            int this_nbjet = 0;
            vector<unsigned int> bindex;
            for (unsigned int iJet = 0; iJet < jets_p4().size(); iJet++) {
                if (jets_p4().at(iJet).pt() < 15.) continue;
                if (jets_simpleSecondaryVertexHighEffBJetTag().at(iJet) < 1.74) continue;
                this_nbjet++;
                bindex.push_back(iJet);
            }

            // PF Jets
            int this_nbpfjet = 0;
            vector<unsigned int> bpfindex;
            for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                if ( !passesPFJetID(iJet)) continue;
                LorentzVector jp4 = pfjets_p4()[iJet];
                float jet_cor = jetCorrection(jp4, jet_pf_L2L3corrector);
                LorentzVector jp4cor = jp4 * jet_cor;
                if (jp4cor.pt() < 15) continue;
                if (pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) < 1.74) continue;
                this_nbpfjet++;
                bpfindex.push_back(iJet);
            }

            // Electrons
            if (eormu == -1 || eormu==11) {
                for (unsigned int iLep = 0 ; iLep < els_p4().size(); iLep++) {

                    // Apply a pt cut (Changed it from 5 GeV to 10 GeV...Claudio 10 July 2010)
                    if ( els_p4().at(iLep).pt() < 10.) continue;

                    // Initialize baby ntuple
                    InitBabyNtuple();

                    //////////////////////////////////////////////////////
                    // Fake Rate Numerator & Denominator Selections     //
                    //////////////////////////////////////////////////////

                    // store number of electron FOs in event (use SS FO definition)
                    nFOels_ = 0;
                    ngsfs_ = 0;
                    for (unsigned int iel = 0; iel < cms2.els_p4().size(); iel++)
                    {
                        if (iel == iLep)
                            continue;

                        if (cms2.els_p4().at(iel).pt() < 10.)
                            continue;

                        if (pass_electronSelection(iel, electronSelectionFOV6_ssVBTF80_v3, false, false))
                            ++ngsfs_;

                        if (samesign::isDenominatorLepton(11, iel, samesign::DET_ISO)) {
                            ++nFOels_;
                            if (cms2.els_p4().at(iel).pt() > foel_p4_.pt() && iel != iLep) {
                                foel_p4_ = cms2.els_p4().at(iel);
                                foel_id_ = 11*cms2.els_charge().at(iel);
                            }
                            continue;
                        }
                        if (samesign::isDenominatorLepton(11, iel, samesign::COR_DET_ISO)) {
                            ++nFOels_;
                            if (cms2.els_p4().at(iel).pt() > foel_p4_.pt() && iel != iLep) {
                                foel_p4_ = cms2.els_p4().at(iel);
                                foel_id_ = 11*cms2.els_charge().at(iel);
                            }
                            continue;
                        }
                    }

                    // store number of muon FOs in event (use SS FO definition)
                    nFOmus_ = 0;
                    nmus_ = 0;
                    for (unsigned int imu = 0; imu < cms2.mus_p4().size(); imu++)
                    {
                        if (cms2.mus_p4().at(imu).pt() < 10.)
                            continue;

                        if (muonIdNotIsolated(imu, NominalSSv4))
                            ++nmus_;

                        if (samesign::isDenominatorLepton(13, imu, samesign::DET_ISO)) {
                            ++nFOmus_;
                            if (cms2.mus_p4().at(imu).pt() > fomu_p4_.pt()) {
                                fomu_p4_ = cms2.mus_p4().at(imu);
                                fomu_id_ = 13*cms2.mus_charge().at(imu);
                            }
                            continue;
                        }
                        if (samesign::isDenominatorLepton(13, imu, samesign::COR_DET_ISO)) {
                            ++nFOmus_;
                            if (cms2.mus_p4().at(imu).pt() > fomu_p4_.pt()) {
                                fomu_p4_ = cms2.mus_p4().at(imu);
                                fomu_id_ = 13*cms2.mus_charge().at(imu);
                            }
                            continue;
                        }
                    }

                    //////////
                    // 2012 //
                    //////////

                    // SS
                    num_el_ssV7_       = pass_electronSelection(iLep, electronSelection_ssV7       );
                    num_el_ssV7_noIso_ = pass_electronSelection(iLep, electronSelection_ssV7_noIso );
                    v1_el_ssV7_        = pass_electronSelection(iLep, electronSelectionFOV7_v1     );
                    v2_el_ssV7_        = pass_electronSelection(iLep, electronSelectionFOV7_v2     );
                    v3_el_ssV7_        = pass_electronSelection(iLep, electronSelectionFOV7_v3     );

                    //////////
                    // 2011 //
                    //////////

                    // SS
                    num_el_ssV6_       = pass_electronSelection( iLep, electronSelection_ssV6            );
                    num_el_ssV6_noIso_ = pass_electronSelection( iLep, electronSelection_ssV6_noIso      );
                    v1_el_ssV6_        = pass_electronSelection( iLep, electronSelectionFOV6_ssVBTF80_v1 );
                    v2_el_ssV6_        = pass_electronSelection( iLep, electronSelectionFOV6_ssVBTF80_v2 );
                    v3_el_ssV6_        = pass_electronSelection( iLep, electronSelectionFOV6_ssVBTF80_v3 );

                    // WW
                    num_el_smurfV6_ = pass_electronSelection( iLep, electronSelection_smurfV6          );
                    v1_el_smurfV1_  = pass_electronSelection( iLep, electronSelectionFO_el_smurf_v1    );
                    v2_el_smurfV1_  = pass_electronSelection( iLep, electronSelectionFO_el_smurf_v2    );
                    v3_el_smurfV1_  = pass_electronSelection( iLep, electronSelectionFO_el_smurf_v3    );
                    v4_el_smurfV1_  = pass_electronSelection( iLep, electronSelectionFO_el_smurf_v4    );
                    
                    //OS
                    num_el_OSV2_   = pass_electronSelection( iLep, electronSelection_el_OSV2          );
                    fo_el_OSV2_    = pass_electronSelection( iLep, electronSelection_el_OSV2_FO       );
                    num_el_OSV3_   = pass_electronSelection( iLep, electronSelection_el_OSV3          );
                    fo_el_OSV3_    = pass_electronSelection( iLep, electronSelection_el_OSV3_FO       );

                    ////////////////////////////////////////////////////////////
                    // Skip this muon if it fails the loosest denominator.    //
                    // Ignore this OR if applyFOfilter is set to false.       //
                    ////////////////////////////////////////////////////////////
                    if (applyFOfilter) {  
                        if (
                            !v1_el_ssV7_    && !v2_el_ssV7_    && !v3_el_ssV7_    &&                    // SS 2012
                            !v1_el_ssV6_    && !v2_el_ssV6_    && !v3_el_ssV6_    &&                    // SS 2011
                            !fo_el_OSV2_    && !fo_el_OSV3_    &&                                       // OS 2011
                            !v1_el_smurfV1_ && !v1_el_smurfV1_ && !v3_el_smurfV1_ && !v4_el_smurfV1_    // WW 2011
                            ) 
                            continue;
                    }
 
                    //////////////////////////////////////////////////////
                    // End Fake Rate Numerator & Denominator Selections //
                    //////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// NEED TO THINK ABOUT THIS... Z'S ARE VETOED BASED ON TOP SELECTIONS //
////////////////////////////////////////////////////////////////////////
       
                    // If it is above 20 GeV see if we can make a 
                    // Z with another pt>20 FO.  Will use the v1 FO since 
                    // these are the loosest
                    bool isaZ = false;
                    if (els_p4().at(iLep).pt() > 20.) {
                        for (unsigned int jEl = 0 ; jEl < els_p4().size(); jEl++) {
                            if (iLep == jEl)                             continue;
                            if (els_p4().at(jEl).pt() < 20.)            continue;
                            if ( ! pass_electronSelection( jEl, electronSelection_el_OSV3_FO ) ) continue;
                            if ( ! fo_el_OSV3_ ) continue;
                            LorentzVector w = els_p4().at(iLep) + els_p4().at(jEl);
                            if (abs(w.mass()-91.) > 20.) continue;
                            isaZ = true;
                        }
                    }
                    if (isaZ) continue;
    
////////////////////////////////////////////////////////////////////////
// STORE SOME Z MASS VARIABLES //
////////////////////////////////////////////////////////////////////////
                    mz_fo_gsf_  = -999.;
                    mz_gsf_iso_ = -999.;
                    LorentzVector p4fo = cms2.els_p4().at(iLep);
                    for (unsigned int iel = 0; iel < cms2.els_p4().size(); iel++) {
                        if (iel == iLep) continue;

                        if (fabs(cms2.els_p4().at(iel).eta()) > 2.5)
                            continue;

                        if (cms2.els_p4().at(iel).pt() < 10.)
                            continue;

                        LorentzVector zp4 = p4fo + cms2.els_p4().at(iel);
                        float zcandmass = sqrt(fabs(zp4.mass2()));
                        if ( fabs(zcandmass - 91.) > fabs(mz_fo_gsf_ - 91.) )
                            continue;

                        mz_fo_gsf_  = zcandmass;
                        mz_gsf_iso_ = electronIsolation_rel_v1(iel, true);
                    }

                    mz_fo_ctf_  = -999.;
                    mz_ctf_iso_ = -999.;
                    for (int ictf = 0; ictf < static_cast<int>(cms2.trks_trk_p4().size()); ictf++) {
                        if (ictf == cms2.els_trkidx().at(iLep)) continue;

                        if (fabs(cms2.trks_trk_p4().at(ictf).eta()) > 2.5)
                            continue;

                        if (cms2.trks_trk_p4().at(ictf).pt() < 10.)
                            continue;

                        LorentzVector zp4 = p4fo + cms2.trks_trk_p4().at(ictf);
                        float zcandmass = sqrt(fabs(zp4.mass2()));
                        if ( fabs(zcandmass - 91.) > fabs(mz_fo_ctf_ - 91.) )
                            continue;
                        
                        mz_fo_ctf_  = zcandmass;
                        mz_ctf_iso_ = ctfIsoValuePF(ictf, associateTrackToVertex(ictf));
                    }


                    /////////////////////////// 
                    // Event Information     //
                    ///////////////////////////

                    // Load the electron and event quantities
                    run_   = evt_run();
                    ls_    = evt_lumiBlock();
                    evt_   = evt_event();
                    weight_ = isData ? 1. : evt_scale1fb();
  
                    if(!isData){
                        // Pileup - PUSummaryInfoMaker                        
                        for (unsigned int vidx = 0; vidx < cms2.puInfo_nPUvertices().size(); vidx++) {
                            if (cms2.puInfo_bunchCrossing().at(vidx) != 0)
                                continue;
                            pu_nPUvertices_ = cms2.puInfo_nPUvertices().at(vidx);
                        }

                    }

                    // Pileup - VertexMaker
                    bool first_good_vertex_found         = false;
                    unsigned int first_good_vertex_index = 0;
                    for (unsigned int vidx = 0; vidx < cms2.vtxs_position().size(); vidx++)
					{
                        if (!isGoodVertex(vidx))
                        {
                            continue;
                        }
						if (!first_good_vertex_found)
						{
							first_good_vertex_found = true;
							first_good_vertex_index = vidx;
						}
                       	++evt_nvtxs_;
                    }
    
                    // Pileup - VertexMaker
                    for (unsigned int vidx = 0; vidx < cms2.davtxs_position().size(); vidx++) {
                        if (!isGoodDAVertex(vidx))
                            continue;
                            
                        ++evt_ndavtxs_;
                    }

                    /////////////////////////// 
                    // End Event Information //
                    ///////////////////////////



                    //////////////////////////// 
                    // Lepton Information     //
                    ////////////////////////////

                    // Basic Quantities
                    lp4_ = cms2.els_p4().at(iLep);
                    pt_             = els_p4().at(iLep).pt();
                    eta_            = els_p4().at(iLep).eta();
                    sceta_          = els_etaSC().at(iLep);
                    phi_            = els_p4().at(iLep).phi();
                    scet_           = els_eSC()[iLep] / cosh( els_etaSC()[iLep] );
                    hoe_            = els_hOverE().at(iLep);
                    id_             = 11*els_charge().at(iLep);
                    pfmet_          = evt_pfmet();
                    pfmetphi_       = evt_pfmetPhi();
                    foel_mass_ = sqrt(fabs((lp4_ + foel_p4_).mass2()));
                    fomu_mass_ = sqrt(fabs((lp4_ + fomu_p4_).mass2()));

                    // Isolation
                    iso_             = electronIsolation_rel         (iLep, true );
                    iso_nps_         = electronIsolation_rel         (iLep, true );  // wrong
                    nt_iso_          = electronIsolation_rel_v1      (iLep, true );
                    nt_iso_nps_      = electronIsolation_rel_v1      (iLep, true );
                    trck_iso_        = electronIsolation_rel         (iLep, false);
                    trck_nt_iso_     = electronIsolation_rel_v1      (iLep, false);
                    ecal_iso_        = electronIsolation_ECAL_rel    (iLep       );
                    ecal_iso_nps_    = electronIsolation_ECAL_rel    (iLep       );  // wrong
                    ecal_nt_iso_     = electronIsolation_ECAL_rel_v1 (iLep, true );
                    ecal_nt_iso_nps_ = electronIsolation_ECAL_rel_v1 (iLep, false);
                    hcal_iso_        = electronIsolation_HCAL_rel    (iLep       );
                    hcal_nt_iso_     = electronIsolation_HCAL_rel_v1 (iLep       );

                    // PF Isolation
                    electronIsoValuePF2012(ch_nt_pfiso03_, em_nt_pfiso03_, nh_nt_pfiso03_, 0.3, iLep, first_good_vertex_index);
                    nt_pfiso03_ = (ch_nt_pfiso03_ + em_nt_pfiso03_ + nh_nt_pfiso03_)/els_p4().at(iLep).pt(); 
                    electronIsoValuePF2012(ch_nt_pfiso04_, em_nt_pfiso04_, nh_nt_pfiso04_, 0.4, iLep, first_good_vertex_index);
                    nt_pfiso04_ = (ch_nt_pfiso04_ + em_nt_pfiso04_ + nh_nt_pfiso04_)/els_p4().at(iLep).pt(); 

                    // PF Isolation
                    electronIsoValuePF2012(ch_nt_pfiso03_bv_, em_nt_pfiso03_bv_, nh_nt_pfiso03_bv_, 0.3, iLep, first_good_vertex_index, /*barrelVetoes=*/true);
                    nt_pfiso03_bv_ = (ch_nt_pfiso03_bv_ + em_nt_pfiso03_bv_ + nh_nt_pfiso03_bv_)/els_p4().at(iLep).pt(); 
                    electronIsoValuePF2012(ch_nt_pfiso04_bv_, em_nt_pfiso04_bv_, nh_nt_pfiso04_bv_, 0.4, iLep, first_good_vertex_index, /*barrelVetoes=*/true);
                    nt_pfiso04_bv_ = (ch_nt_pfiso04_bv_ + em_nt_pfiso04_bv_ + nh_nt_pfiso04_bv_)/els_p4().at(iLep).pt(); 

                    // mc information
                    if (!isData) {
                        mcid_       = els_mc_id().at(iLep);
                        mcmotherid_ = els_mc_motherid().at(iLep);
                        int status3_index = mc3idx_eormu(11, iLep);
                        if (status3_index >= 0)
                        {
                            mc3id_ = cms2.genps_id().at(status3_index);
                            mc3pt_ = cms2.genps_p4().at(status3_index).pt();                            
                            mc3p4_ = cms2.genps_p4().at(status3_index);                            
                        }
                        mc3dr_ = mc3dr_eormu(11, iLep);            
                        leptonIsFromW_ = leptonIsFromW(iLep, -11 * cms2.els_charge().at(iLep), true);
                    }

                    // ID
                    el_id_smurfV5_ = pass_electronSelection( iLep, electronSelection_smurfV5_id );
                    el_id_vbtf80_  = electronId_VBTF(iLep, VBTF_35X_80, false, false);
                    el_id_vbtf90_  = electronId_VBTF(iLep, VBTF_35X_90, false, false);
                    if( els_closestMuon().at(iLep) == -1 )
                        closestMuon_ = true;

                    // electron ID effective area
                    // 2012 working point effective id (taken From electronSelections.h -- electronId_WP2012()) 
                    el_id_effarea_ = 0.18;
                    if (fabs(eta_ ) > 1.0   && fabs(eta_) <= 1.479 ) { el_id_effarea_ = 0.19; }
                    if (fabs(eta_ ) > 1.479 && fabs(eta_) <= 2.0   ) { el_id_effarea_ = 0.21; }
                    if (fabs(eta_ ) > 2.0   && fabs(eta_) <= 2.2   ) { el_id_effarea_ = 0.38; }
                    if (fabs(eta_ ) > 2.2   && fabs(eta_) <= 2.3   ) { el_id_effarea_ = 0.61; }
                    if (fabs(eta_ ) > 2.3   && fabs(eta_) <= 2.4   ) { el_id_effarea_ = 0.73; }
                    if (fabs(eta_) > 2.4)                            { el_id_effarea_ = 0.78; }
                    
                    // PV
                    d0PV_wwV1_ = electron_d0PV_wwV1(iLep);
                    dzPV_wwV1_ = electron_dzPV_wwV1(iLep);

                    // W transverse mass
                    mt_   = Mt( els_p4().at(iLep), pfmet_, pfmetphi_ );
                    pfmt_ = Mt( els_p4().at(iLep), pfmet_, pfmetphi_ );

                    // Do the 3 electron charges agree?
                    int iCTF = els_trkidx().at(iLep);
                    if( iCTF >= 0 ){
                        int qCTF = trks_charge().at( iCTF );
                        int qGSF = els_trk_charge().at(iLep);
                        int qPIX = els_sccharge().at(iLep);
                        if( qCTF == qGSF && qCTF == qPIX && qGSF == qPIX ) q3_ = true;
                    }
  
                    // Missing hits info
                    els_exp_innerlayers_ = els_exp_innerlayers().at(iLep);

                    // Conversion Rejection  
                    convHitPattern_   = isFromConversionHitPattern(iLep);
                    convPartnerTrack_ = isFromConversionPartnerTrack(iLep);
                    convMIT_          = isFromConversionMIT(iLep);
                    if( els_exp_innerlayers().at(iLep) == 0 ) conv0MissHits_ = true;

                    // HT
                    ht_calo_           = (float) sumPt (iLep, JETS_TYPE_CALO_UNCORR  , JETS_CLEAN_SINGLE_E );
                    ht_calo_L2L3_      = (float) sumPt (iLep, JETS_TYPE_CALO_CORR    , JETS_CLEAN_SINGLE_E );
                    ht_pf_             = (float) sumPt (iLep, JETS_TYPE_PF_UNCORR    , JETS_CLEAN_SINGLE_E );
                    ht_pf_L2L3_        = (float) sumPt (iLep, JETS_TYPE_PF_CORR      , JETS_CLEAN_SINGLE_E );
                    //ht_pf_L1FastL2L3_  = (float) sumPt (iLep, JETS_TYPE_PF_FAST_CORR , JETS_CLEAN_SINGLE_E );

                    //////////////////////////// 
                    // End Lepton Information //
                    ////////////////////////////

                    ///////////////////////  
                    // 2012 Triggers     //
                    ///////////////////////

                    // Electrons
                    triggerMatchStruct struct_ele8_CaloIdL_CaloIsoVL_vstar                                = MatchTriggerClass(els_p4().at(iLep), &ele8_CaloIdL_CaloIsoVL_regexp                               );
                    triggerMatchStruct struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar               = MatchTriggerClass(els_p4().at(iLep), &ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_regexp              );
                    triggerMatchStruct struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar         = MatchTriggerClass(els_p4().at(iLep), &ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_regexp        );
                    triggerMatchStruct struct_ele8_CaloIdT_TrkIdVL_vstar                                  = MatchTriggerClass(els_p4().at(iLep), &ele8_CaloIdT_TrkIdVL_regexp                                 );
                    triggerMatchStruct struct_ele17_CaloIdL_CaloIsoVL_vstar                               = MatchTriggerClass(els_p4().at(iLep), &ele17_CaloIdL_CaloIsoVL_regexp                              );
                    triggerMatchStruct struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar              = MatchTriggerClass(els_p4().at(iLep), &ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_regexp             );
                    triggerMatchStruct struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar        = MatchTriggerClass(els_p4().at(iLep), &ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_regexp       );
                    triggerMatchStruct struct_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar = MatchTriggerClass(els_p4().at(iLep), &ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_rexexp);
                    triggerMatchStruct struct_ele27_WP80_vstar                                            = MatchTriggerClass(els_p4().at(iLep), &ele27_WP80_rexexp                                           );

                    ele8_CaloIdL_CaloIsoVL_vstar_                                         = struct_ele8_CaloIdL_CaloIsoVL_vstar.nHLTObjects_;
                    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                        = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.nHLTObjects_;
                    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_                  = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.nHLTObjects_;
                    ele8_CaloIdT_TrkIdVL_vstar_                                           = struct_ele8_CaloIdT_TrkIdVL_vstar.nHLTObjects_;
                    ele17_CaloIdL_CaloIsoVL_vstar_                                        = struct_ele17_CaloIdL_CaloIsoVL_vstar.nHLTObjects_;
                    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                       = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.nHLTObjects_;
                    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_                 = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.nHLTObjects_;
                    ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_          = struct_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar.nHLTObjects_;
                    ele27_WP80_vstar_                                                     = struct_ele27_WP80_vstar.nHLTObjects_;

                    ele8_CaloIdL_CaloIsoVL_version_                                       = struct_ele8_CaloIdL_CaloIsoVL_vstar.version_;
                    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version_                      = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.version_;
                    ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version_                = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.version_;
                    ele8_CaloIdT_TrkIdVL_version_                                         = struct_ele8_CaloIdT_TrkIdVL_vstar.version_;
                    ele17_CaloIdL_CaloIsoVL_version_                                      = struct_ele17_CaloIdL_CaloIsoVL_vstar.version_;
                    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_version_                     = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.version_;
                    ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_version_               = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.version_;
                    ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_version_        = struct_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar.version_;
                    ele27_WP80_version_                                                   = struct_ele27_WP80_vstar.version_;

                    dr_ele8_CaloIdL_CaloIsoVL_vstar_                                      = struct_ele8_CaloIdL_CaloIsoVL_vstar.dR_;
                    dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                     = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.dR_;
                    dr_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_               = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.dR_;
                    dr_ele8_CaloIdT_TrkIdVL_vstar_                                        = struct_ele8_CaloIdT_TrkIdVL_vstar.dR_;
                    dr_ele17_CaloIdL_CaloIsoVL_vstar_                                     = struct_ele17_CaloIdL_CaloIsoVL_vstar.dR_;
                    dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                    = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.dR_;
                    dr_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_              = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.dR_;
                    dr_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_       = struct_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar.dR_;
                    dr_ele27_WP80_vstar_                                                  = struct_ele27_WP80_vstar.dR_;

                    hltps_ele8_CaloIdL_CaloIsoVL_vstar_                                   = struct_ele8_CaloIdL_CaloIsoVL_vstar.hltps_;
                    hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                  = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.hltps_;
                    hltps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_            = struct_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.hltps_;
                    hltps_ele8_CaloIdT_TrkIdVL_vstar_                                     = struct_ele8_CaloIdT_TrkIdVL_vstar.hltps_;
                    hltps_ele17_CaloIdL_CaloIsoVL_vstar_                                  = struct_ele17_CaloIdL_CaloIsoVL_vstar.hltps_;
                    hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                 = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar.hltps_;
                    hltps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_           = struct_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar.hltps_;
                    hltps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_    = struct_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar.hltps_;
                    hltps_ele27_WP80_vstar_                                               = struct_ele27_WP80_vstar.hltps_;

                    // These are hardcoded to the value in Dima's table:
                    // http://dmytro.web.cern.ch/dmytro/trigger/triggerEvolution_all.html
                    l1ps_ele8_CaloIdL_CaloIsoVL_vstar_                                    = L1_prescale("L1_SingleEG5");
                    l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                   = L1_prescale("L1_SingleEG7");
                    l1ps_ele8_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_             = L1_prescale("L1_SingleEG7");
                    l1ps_ele8_CaloIdT_TrkIdVL_vstar_                                      = L1_prescale("L1_SingleEG5");
                    l1ps_ele17_CaloIdL_CaloIsoVL_vstar_                                   = L1_prescale("L1_SingleEG12");
                    l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_vstar_                  = L1_prescale("L1_SingleEG12");
                    l1ps_ele17_CaloIdT_CaloIsoVL_TrkIdVL_TrkIsoVL_Jet30_vstar_            = L1_prescale("L1_SingleEG12");
                    l1ps_ele25_CaloIdVT_CaloIsoT_TrkIdT_TrkIsoT_CentralPFJet30_vstar_     = L1_prescale("L1_SingleEG20");
                    l1ps_ele27_WP80_vstar_                                                = L1_prescale("L1_SingleEG20");

                    ///////////////////////  
                    // end 2012 Triggers //
                    ///////////////////////

                    ///////////////////////  
                    // 2011 Triggers     //
                    ///////////////////////
                    

                    // Electrons
                    triggerMatchStruct struct_ele8_vstar                                          = MatchTriggerClass( els_p4().at(iLep), &ele8_regexp                                         );
                    triggerMatchStruct struct_ele8_CaloIdL_TrkIdVL_vstar                          = MatchTriggerClass( els_p4().at(iLep), &ele8_CaloIdL_TrkIdVL_regexp                         );
                    triggerMatchStruct struct_ele8_CaloIdL_CaloIsoVL_Jet40_vstar                  = MatchTriggerClass( els_p4().at(iLep), &ele8_CaloIdL_CaloIsoVL_Jet40_regexp                 );
                    triggerMatchStruct struct_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar       = MatchTriggerClass( els_p4().at(iLep), &ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_regexp);
                    triggerMatchStruct struct_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar = MatchTriggerClass( els_p4().at(iLep), &photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_regexp);

                    ele8_vstar_                                             = struct_ele8_vstar.nHLTObjects_;
                    ele8_CaloIdL_TrkIdVL_vstar_                             = struct_ele8_CaloIdL_TrkIdVL_vstar.nHLTObjects_; 
                    ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                     = struct_ele8_CaloIdL_CaloIsoVL_Jet40_vstar.nHLTObjects_;
                    ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_          = struct_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar.nHLTObjects_;
                    photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_    = struct_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar.nHLTObjects_;

                    ele8_version_                                           = struct_ele8_vstar.version_;
                    ele8_CaloIdL_TrkIdVL_version_                           = struct_ele8_CaloIdL_TrkIdVL_vstar.version_; 
                    ele8_CaloIdL_CaloIsoVL_Jet40_version_                   = struct_ele8_CaloIdL_CaloIsoVL_Jet40_vstar.version_;
                    ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_version_        = struct_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar.version_;
                    photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_version_  = struct_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar.nHLTObjects_;

                    dr_ele8_vstar_                                          = struct_ele8_vstar.dR_;
                    dr_ele8_CaloIdL_TrkIdVL_vstar_                          = struct_ele8_CaloIdL_TrkIdVL_vstar.dR_;
                    dr_ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                  = struct_ele8_CaloIdL_CaloIsoVL_Jet40_vstar.dR_;
                    dr_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_       = struct_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar.dR_;
                    dr_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_ = struct_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar.dR_; 

                    hltps_ele8_vstar_                                          = struct_ele8_vstar.hltps_;
                    hltps_ele8_CaloIdL_TrkIdVL_vstar_                          = struct_ele8_CaloIdL_TrkIdVL_vstar.hltps_;
                    hltps_ele8_CaloIdL_CaloIsoVL_Jet40_vstar_                  = struct_ele8_CaloIdL_CaloIsoVL_Jet40_vstar.hltps_;
                    hltps_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar_       = struct_ele8_CaloIdT_TrkIdVL_CaloIsoVL_TrkIsoVL_vstar.hltps_;
                    hltps_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar_ = struct_photon20_CaloIdVT_IsoT_Ele8_CaloIdL_CaloIsoVL_vstar.hltps_; 

                    ///////////////////////  
                    // end 2011 Triggers //
                    ///////////////////////

                    //////////////
                    // Jets     //
                    //////////////

                    // Calo Jets
                    // Find the highest Pt jet separated by at least dRcut from this lepton and fill the jet Pt
                    ptj1_       = -999.0;
                    ptj1_b2b_   = -999.0;
                    dphij1_b2b_ = -999.0;
                    nj1_        = 0;
                    for (unsigned int iJet = 0; iJet < jets_p4().size(); iJet++) {
                        double dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), jets_p4().at(iJet) );
                        if( dr > deltaRCut && jets_p4().at(iJet).pt() > 10 ) nj1_++;
                        if ( dr > deltaRCut && jets_p4().at(iJet).pt() > ptj1_ ){
                            ptj1_ = jets_p4().at(iJet).pt();
          
                            // back to back in phi
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( els_p4().at(iLep), jets_p4().at(iJet) ) );
                            if( dphi > deltaPhiCut && jets_p4().at(iJet).pt() > ptj1_b2b_ ){ 
                                ptj1_b2b_   = jets_p4().at(iJet).pt();
                                dphij1_b2b_ = dphi;
                            }
                        }
                    }
    
                    // PF Jets
                    // Find the highest Pt pfjet separated by at least dRcut from this lepton and fill the pfjet Pt
                    ptpfj1_       = -999.0;
                    ptpfj1_b2b_   = -999.0;
                    dphipfj1_b2b_ = -999.0;
                    npfj1_        = 0;
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        double dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), pfjets_p4().at(iJet) );
                        if( dr > deltaRCut && pfjets_p4().at(iJet).pt() > 10 ) npfj1_++;
                        if ( dr > deltaRCut && pfjets_p4().at(iJet).pt() > ptpfj1_ ){
                            ptpfj1_ = pfjets_p4().at(iJet).pt();
          
                            // back to back in phi
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( els_p4().at(iLep), pfjets_p4().at(iJet) ) );
                            if( dphi > deltaPhiCut && pfjets_p4().at(iJet).pt() > ptpfj1_b2b_ ){ 
                                ptpfj1_b2b_   = pfjets_p4().at(iJet).pt();
                                dphij1_b2b_ = dphi;
                            }
                        }
                    }
    
                    // L2L3 PF Jets
                    // Find the highest Pt PF L2L3 corrected jet separated by at least dRcut from this lepton and fill the jet Pt
                    ptpfcj1_       = -999.0; 
                    ptpfcj1_b2b_   = -999.0;
                    dphipfcj1_b2b_ = -999.0;
                    npfcj1_        = 0;
                    btagpfc_       = false;
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        if ( !passesPFJetID(iJet)) continue;
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = jetCorrection(jp4, jet_pf_L2L3corrector);
                        LorentzVector jp4cor = jp4 * jet_cor;
                        if (jp4cor.pt() > 15 && pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) > 1.74 ) btagpfc_ = true;
                        double dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), jp4cor );
                        if( dr > deltaRCut && jp4cor.pt() > 10 ) npfcj1_++;
                        if ( dr > deltaRCut && jp4cor.pt() > ptpfcj1_ ){
                            ptpfcj1_ = jp4cor.pt();
    
                            // back to back in phi
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( els_p4().at(iLep), jp4cor ) );
                            if( dphi > deltaPhiCut && jp4cor.pt() > ptpfcj1_b2b_ ){
                                ptpfcj1_b2b_   = jp4cor.pt();
                                dphipfcj1_b2b_ = dphi;
                            } 
                        }
                    }

                    // L1FastL2L3 PF Jets
                    // Find the highest Pt PF L1FastL2L3 corrected jet separated by at least dRcut from this lepton and fill the jet Pt
                    emfpfcL1Fj1_      = -999.0;
                    ptpfcL1Fj1_       = -999.0;
                    dphipfcL1Fj1_ 	  = -999.0;
                    ptpfcL1Fj1_b2b_   = -999.0;
                    dphipfcL1Fj1_b2b_ = -999.0;
                    npfcL1Fj1_        = 0;
                    npfc30L1Fj1_      = 0;
                    npfc30L1Fj1_      = 0;
                    btagpfcL1F_       = false;
                    rho_ = cms2.evt_rho();
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        if ( !passesPFJetID(iJet)) continue;
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = cms2.pfjets_corL1FastL2L3()[iJet];
                        LorentzVector jp4cor = jp4 * jet_cor;
                        if (jp4cor.pt() > 15 && pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) > 1.74 ) btagpfcL1F_ = true;
                        double dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), jp4cor );
                        if( dr > deltaRCut && jp4cor.pt() > 10 ) npfcL1Fj1_++;
                        if( dr > deltaRCut && jp4cor.pt() > 30 ) npfc30L1Fj1_++;
                        if( dr > deltaRCut && jp4cor.pt() > 40 ) npfc40L1Fj1_++;
                        if ( dr > deltaRCut && jp4cor.pt() > ptpfcL1Fj1_ ){
                            emfpfcL1Fj1_ = (cms2.pfjets_chargedEmE().at(iJet) + cms2.pfjets_neutralEmE().at(iJet)) / pfjets_p4().at(iJet).E();
                            ptpfcL1Fj1_ = jp4cor.pt();
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( els_p4().at(iLep), jp4cor ) );
                            dphipfcL1Fj1_ = dphi;

                            // back to back in phi
                            if( dphi > deltaPhiCut && jp4cor.pt() > ptpfcL1Fj1_b2b_ ){
                                ptpfcL1Fj1_b2b_   = jp4cor.pt();
                                dphipfcL1Fj1_b2b_ = dphi;
                            }
                        }
                    }

					// *** Doing B-tagging correctly ***
                    // B-tagged L1FastL2L3 PF Jets
                    // Find the highest Pt B-tagged PF L1FastL2L3 corrected jet separated by at least dRcut from this lepton and fill the jet Pt
                    ptbtagpfcL1Fj1_       = -999.0;
                    dphibtagpfcL1Fj1_       = -999.0;
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        if ( !passesPFJetID(iJet)) continue;
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = cms2.pfjets_corL1FastL2L3()[iJet];
                        LorentzVector jp4cor = jp4 * jet_cor;
                        if (pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) < 1.74 ) continue;
                        double dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), jp4cor );
                        if ( dr > deltaRCut && jp4cor.pt() > ptbtagpfcL1Fj1_ ){
                            ptbtagpfcL1Fj1_ = jp4cor.pt();
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( els_p4().at(iLep), jp4cor ) );
							dphibtagpfcL1Fj1_ = dphi; 
                        }
                    }

                    //////////////
                    // End Jets //
                    //////////////


                    ///////////////////
                    // B Tagging     //
                    ///////////////////

                    // The btag information
                    nbjet_ = this_nbjet;
                    dRbNear_ = 99.;
                    dRbFar_  = -99.;
                    for (int ii=0; ii<nbjet_; ii++) {
                        unsigned int iJet = bindex[ii];
                        float dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), jets_p4().at(iJet));
                        if (dr < dRbNear_) dRbNear_ = dr;
                        if (dr > dRbFar_)   dRbFar_  = dr;
                    }

                    // btag info for corrected pfjet
                    nbpfcjet_ = this_nbpfjet;
                    dRbpfcNear_ = 99.;
                    dRbpfcFar_  = -99.;
                    for (int ii=0; ii<nbpfcjet_; ii++) {
                        unsigned int iJet = bpfindex[ii];
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = jetCorrection(jp4, jet_pf_L2L3corrector);
                        LorentzVector jp4cor = jp4 * jet_cor;
                        float dr = ROOT::Math::VectorUtil::DeltaR( els_p4().at(iLep), jp4cor);
                        if (dr < dRbpfcNear_) dRbpfcNear_ = dr;
                        if (dr > dRbpfcFar_)   dRbpfcFar_  = dr;
                    }

                    ///////////////////
                    // End B Tagging //
                    ///////////////////



                    // Time to fill the baby for the electrons
                    FillBabyNtuple();

                } // closes loop over electrons
            } // closes if statements about whether we want to fill electrons


            // Muons
            if (eormu == -1 || eormu==13) {
                for ( unsigned int iLep = 0; iLep < mus_p4().size(); iLep++) {

                    // Apply a pt cut
                    if ( mus_p4().at(iLep).pt() < 5.0) continue;
        
////////////////////////////////////////////////////////////////////////
// NEED TO THINK ABOUT THIS... Z'S ARE VETOED BASED ON TOP SELECTIONS //
////////////////////////////////////////////////////////////////////////

                    // If it is above 20 GeV see if we can make a 
                    // Z with another pt>20 FO.  
                    bool isaZ = false;
                    if (mus_p4().at(iLep).pt() > 20.) {
                        for (unsigned int jMu = 0 ; jMu < mus_p4().size(); jMu++) {
                            if (iLep == jMu)                             continue;
                            if (mus_p4().at(jMu).pt() < 20.)            continue;
                            if ( ! muonId( jMu,  OSGeneric_v3_FO) ) continue;
                            if ( ! muonId( iLep, OSGeneric_v3_FO) ) continue;
                            LorentzVector w = mus_p4().at(iLep) + mus_p4().at(jMu);
                            if (abs(w.mass()-91.) > 20.) continue;
                            isaZ = true;
                        }
                    }
                    if (isaZ) continue;
        
                    // Initialize baby ntuple
                    InitBabyNtuple();
 
                    // store number of electron FOs in event (use SS FO definition)
                    nFOels_ = 0;
                    ngsfs_ = 0;
                    for (unsigned int iel = 0; iel < cms2.els_p4().size(); iel++) {
                        if (cms2.els_p4().at(iel).pt() < 10.)
                            continue;

                        if (pass_electronSelection(iel, electronSelectionFOV6_ssVBTF80_v3, false, false))
                            ++ngsfs_;

                        if (samesign::isDenominatorLepton(11, iel, samesign::DET_ISO)) {
                            ++nFOels_;
                            if (cms2.els_p4().at(iel).pt() > foel_p4_.pt()) {
                                foel_p4_ = cms2.els_p4().at(iel);
                                foel_id_ = 11*cms2.els_charge().at(iel);
                            }
                            continue;
                        }
                        if (samesign::isDenominatorLepton(11, iel, samesign::COR_DET_ISO)) {
                            ++nFOels_;
                            if (cms2.els_p4().at(iel).pt() > foel_p4_.pt()) {
                                foel_p4_ = cms2.els_p4().at(iel);
                                foel_id_ = 11*cms2.els_charge().at(iel);
                            }
                            continue;
                        }
                    }
            
                    // store number of muon FOs in event (use SS FO definition)
                    nFOmus_ = 0;
                    nmus_ = 0;
                    for (unsigned int imu = 0; imu < cms2.mus_p4().size(); imu++) {
                        if (imu == iLep)
                            continue;

                        if (cms2.mus_p4().at(imu).pt() < 10.)
                            continue;

                        if (muonIdNotIsolated(imu, NominalSSv4))
                            ++nmus_;

                        if (samesign::isDenominatorLepton(13, imu, samesign::DET_ISO)) {
                            ++nFOmus_;
                            if (cms2.mus_p4().at(imu).pt() > fomu_p4_.pt() && imu != iLep) {
                                fomu_p4_ = cms2.mus_p4().at(imu);
                                fomu_id_ = 13*cms2.mus_charge().at(imu);
                            }
                            continue;
                        }
                        if (samesign::isDenominatorLepton(13, imu, samesign::COR_DET_ISO)) {
                            ++nFOmus_;
                            if (cms2.mus_p4().at(imu).pt() > fomu_p4_.pt() && imu != iLep) {
                                fomu_p4_ = cms2.mus_p4().at(imu);
                                fomu_id_ = 13*cms2.mus_charge().at(imu);
                            }
                            continue;
                        }
                    }

////////////////////////////////////////////////////////////////////////
// STORE SOME Z MASS VARIABLES //
////////////////////////////////////////////////////////////////////////
//
                    mz_fo_ctf_  = -999.;
                    mz_ctf_iso_ = -999.;
                    mupsilon_fo_mu_ = -999.;
                    mupsilon_mu_iso_ = -999.;
                    LorentzVector p4fo = cms2.mus_p4().at(iLep);
                    for (unsigned int imu = 0; imu < cms2.mus_p4().size(); imu++) {
                        if (imu == iLep) continue;

                        if (fabs(cms2.mus_p4().at(imu).eta()) > 2.5)
                            continue;

                        if (cms2.mus_p4().at(imu).pt() < 10.)
                            continue;

                        LorentzVector zp4 = p4fo + cms2.mus_p4().at(imu);
                        float zcandmass = sqrt(fabs(zp4.mass2()));
                        if ( fabs(zcandmass - 91.) < fabs(mz_fo_ctf_ - 91.) ) {
                            mz_fo_ctf_  = zcandmass;
                            mz_ctf_iso_ = muonIsoValue(imu, false);
                        }
                        if ( fabs(zcandmass - 9.5) < fabs(mupsilon_fo_mu_ - 9.5) ) {
                            mupsilon_fo_mu_  = zcandmass;
                            mupsilon_mu_iso_ = muonIsoValue(imu, false); 
                        }
                    }
       

                    /////////////////////////// 
                    // Event Information     //
                    ///////////////////////////

                    // Load the electron and event quantities
                    run_    = evt_run();
                    ls_     = evt_lumiBlock();
                    evt_    = evt_event();
                    weight_ = isData ? 1. : evt_scale1fb();

                    if(!isData){
                        // Pileup - PUSummaryInfoMaker
                        for (unsigned int vidx = 0; vidx < cms2.puInfo_nPUvertices().size(); vidx++) {
                            if (cms2.puInfo_bunchCrossing().at(vidx) != 0)
                                continue;
                            pu_nPUvertices_ = cms2.puInfo_nPUvertices().at(vidx);
                        }
                    } 

                    // Pileup - VertexMaker
                    bool first_good_vertex_found         = false;
                    unsigned int first_good_vertex_index = 0;
                    for (unsigned int vidx = 0; vidx < cms2.vtxs_position().size(); vidx++)
					{
                        if (!isGoodVertex(vidx))
                        {
                            continue;
                        }
						if (!first_good_vertex_found)
						{
							first_good_vertex_found = true;
							first_good_vertex_index = vidx;
						}
                       	++evt_nvtxs_;
                    }
    
                    // Pileup - VertexMaker
                    for (unsigned int vidx = 0; vidx < cms2.davtxs_position().size(); vidx++) {
                        if (!isGoodDAVertex(vidx))
                            continue;

                        ++evt_ndavtxs_;
                    }

                    /////////////////////////// 
                    // End Event Information //
                    ///////////////////////////



                    //////////////////////////// 
                    // Lepton Information     //
                    ////////////////////////////

                    // Basic Quantities
                    lp4_       = cms2.mus_p4().at(iLep);
                    pt_        = mus_p4().at(iLep).pt();
                    eta_       = mus_p4().at(iLep).eta();
                    phi_       = mus_p4().at(iLep).phi();
                    id_        = 13*mus_charge().at(iLep);
                    pfmet_     = evt_pfmet();
                    pfmetphi_  = evt_pfmetPhi();
                    foel_mass_ = sqrt(fabs((lp4_ + foel_p4_).mass2()));
                    fomu_mass_ = sqrt(fabs((lp4_ + fomu_p4_).mass2()));

                    // Isolation
                    iso_          = muonIsoValue(iLep);
                    nt_iso_       = muonIsoValue(iLep, false);
                    trck_iso_     = muonIsoValue_TRK(iLep);
                    trck_nt_iso_  = muonIsoValue_TRK(iLep, false);
                    ecal_iso_     = muonIsoValue_ECAL(iLep);
                    ecal_nt_iso_  = muonIsoValue_ECAL(iLep, false);
                    hcal_iso_     = muonIsoValue_HCAL(iLep);
                    hcal_nt_iso_  = muonIsoValue_HCAL(iLep, false);

                    // PF Isolation 03
                    ch_nt_pfiso03_ = mus_isoR03_pf_ChargedHadronPt().at(iLep);
                    nh_nt_pfiso03_ = mus_isoR03_pf_NeutralHadronEt().at(iLep);
                    em_nt_pfiso03_ = mus_isoR03_pf_PhotonEt().at(iLep);
                    nt_pfiso03_    = (ch_nt_pfiso03_ + em_nt_pfiso03_ + nh_nt_pfiso03_)/mus_p4().at(iLep).pt(); 

                    // PF Isolation 04
                    ch_nt_pfiso04_ = mus_isoR04_pf_ChargedHadronPt().at(iLep);
                    nh_nt_pfiso04_ = mus_isoR04_pf_NeutralHadronEt().at(iLep);
                    em_nt_pfiso04_ = mus_isoR04_pf_PhotonEt().at(iLep);
                    nt_pfiso04_    = (ch_nt_pfiso04_ + em_nt_pfiso04_ + nh_nt_pfiso04_)/mus_p4().at(iLep).pt(); 

                    // This isn't working yet but we want to use this method for older releases
                    //muonIsoValuePF2012(ch_nt_pfiso03_, em_nt_pfiso03_, nh_nt_pfiso03_, 0.3, iLep, first_good_vertex_index);
                    //muonIsoValuePF2012(ch_nt_pfiso04_, em_nt_pfiso04_, nh_nt_pfiso04_, 0.4, iLep, first_good_vertex_index);
                   
                    // Radial Isolation 03
                    nt_radiso_et1p0_ = muonRadialIsolation(iLep, ch_nt_radiso_et1p0_, nh_nt_radiso_et1p0_, em_nt_radiso_et1p0_, /*neutral_et_threshold=*/1.0, /*cone size=*/0.3, /*verbose=*/false); 
                    nt_radiso_et0p5_ = muonRadialIsolation(iLep, ch_nt_radiso_et0p5_, nh_nt_radiso_et0p5_, em_nt_radiso_et0p5_, /*neutral_et_threshold=*/0.5, /*cone size=*/0.3, /*verbose=*/false); 

                    // mc information
                    if (!isData) 
                    {
                        mcid_       = mus_mc_id().at(iLep);
                        mcmotherid_ = mus_mc_motherid().at(iLep);
                        int status3_index = mc3idx_eormu(13, iLep);
                        if (status3_index >= 0) {
                            mc3id_ = cms2.genps_id().at(status3_index);
                            mc3pt_ = cms2.genps_p4().at(status3_index).pt();
                            mc3p4_ = cms2.genps_p4().at(status3_index);
                        }
                        mc3dr_ = mc3dr_eormu(13, iLep);
                        leptonIsFromW_ = leptonIsFromW(iLep, -13*cms2.mus_charge().at(iLep), true);
                    }

                    mu_isCosmic_ = isCosmics(iLep);

                    // W transverse mass
                    mt_   = Mt( mus_p4().at(iLep), pfmet_, pfmetphi_ );
                    pfmt_ = Mt( mus_p4().at(iLep), pfmet_, pfmetphi_ );

                    // HT
                    ht_calo_           = (float) sumPt (iLep, JETS_TYPE_CALO_UNCORR  , JETS_CLEAN_SINGLE_MU );
                    ht_calo_L2L3_      = (float) sumPt (iLep, JETS_TYPE_CALO_CORR    , JETS_CLEAN_SINGLE_MU );
                    ht_pf_             = (float) sumPt (iLep, JETS_TYPE_PF_UNCORR    , JETS_CLEAN_SINGLE_MU );
                    ht_pf_L2L3_        = (float) sumPt (iLep, JETS_TYPE_PF_CORR      , JETS_CLEAN_SINGLE_MU );

                    //////////////////////////// 
                    // End Lepton Information //
                    ////////////////////////////

                    //////////////////////////////////////////////////////
                    // Fake Rate Numerator & Denominator Selections     //
                    //////////////////////////////////////////////////////

                    //////////
                    // 2012 //
                    //////////

                    // SS
                    num_mu_ssV5_       = muonId(iLep, NominalSSv5);
                    num_mu_ssV5_noIso_ = muonIdNotIsolated(iLep, NominalSSv5);
                    fo_mu_ssV5_        = muonId(iLep, muonSelectionFO_ssV5);
                    fo_mu_ssV5_noIso_  = muonIdNotIsolated(iLep, muonSelectionFO_ssV5);


                    //////////
                    // 2011 //
                    //////////

                    // SS
                    numNomSSv4_      = muonId(iLep, NominalSSv4          );
                    fo_mussV4_04_    = muonId(iLep, muonSelectionFO_ssV4 );
                    numNomSSv4noIso_ = muonIdNotIsolated(iLep, NominalSSv4);
                    fo_mussV4_noIso_ = muonIdNotIsolated(iLep, muonSelectionFO_ssV4 );

                    //OS
                    num_mu_OSGV3_     = muonId(iLep, OSGeneric_v3);
                    fo_mu_OSGV3_      = muonId(iLep, OSGeneric_v3_FO);

                    // WW
                    num_mu_smurfV6_    = muonId(iLep, NominalSmurfV6                    );
                    fo_mu_smurf_04_    = muonId(iLep, muonSelectionFO_mu_smurf_04       );
                    fo_mu_smurf_10_    = muonId(iLep, muonSelectionFO_mu_smurf_10       );

                    ////////////////////////////////////////////////////////////
                    // Skip this muon if it fails the loosest denominator.    //
                    // Ignore this OR if applyFOfilter is set to false.       //
                    ////////////////////////////////////////////////////////////
                    if (applyFOfilter) {
                        if (
                            !fo_mussV4_04_   && !fo_mu_ssV5_     &&                    // SS
                            !fo_mu_OSGV2_    && !fo_mu_OSGV3_    &&                    // OS
                            !fo_mu_smurf_04_ && !fo_mu_smurf_10_                       // WW
                            )
                            continue;
                    }
  
                    //////////////////////////////////////////////////////
                    // End Fake Rate Numerator & Denominator Selections //
                    //////////////////////////////////////////////////////

                    ///////////////////////  
                    // 2012 Triggers     //
                    ///////////////////////

                    // Muons
                    triggerMatchStruct struct_mu5_vstar            = MatchTriggerClass( mus_p4().at(iLep), &mu5_regexp           , 13);
                    triggerMatchStruct struct_mu8_vstar            = MatchTriggerClass( mus_p4().at(iLep), &mu8_regexp           , 13);
                    triggerMatchStruct struct_mu12_vstar           = MatchTriggerClass( mus_p4().at(iLep), &mu12_regexp          , 13);
                    triggerMatchStruct struct_mu17_vstar           = MatchTriggerClass( mus_p4().at(iLep), &mu17_regexp          , 13);
                    triggerMatchStruct struct_mu15_eta2p1_vstar    = MatchTriggerClass( mus_p4().at(iLep), &mu15_eta2p1_regexp   , 13);
                    triggerMatchStruct struct_mu24_eta2p1_vstar    = MatchTriggerClass( mus_p4().at(iLep), &mu24_eta2p1_regexp   , 13);
                    triggerMatchStruct struct_mu30_eta2p1_vstar    = MatchTriggerClass( mus_p4().at(iLep), &mu30_eta2p1_regexp   , 13);
                    triggerMatchStruct struct_isomu20_eta2p1_vstar = MatchTriggerClass( mus_p4().at(iLep), &isomu20_eta2p1_regexp, 13);
                    triggerMatchStruct struct_isomu24_eta2p1_vstar = MatchTriggerClass( mus_p4().at(iLep), &isomu24_eta2p1_regexp, 13);
                    triggerMatchStruct struct_isomu30_eta2p1_vstar = MatchTriggerClass( mus_p4().at(iLep), &isomu30_eta2p1_regexp, 13);

                    mu5_vstar_               = struct_mu5_vstar.nHLTObjects_;
                    mu8_vstar_               = struct_mu8_vstar.nHLTObjects_;
                    mu12_vstar_              = struct_mu12_vstar.nHLTObjects_;
                    mu17_vstar_              = struct_mu17_vstar.nHLTObjects_;
                    mu15_eta2p1_vstar_       = struct_mu15_eta2p1_vstar.nHLTObjects_;
                    mu24_eta2p1_vstar_       = struct_mu24_eta2p1_vstar.nHLTObjects_;
                    mu30_eta2p1_vstar_       = struct_mu30_eta2p1_vstar.nHLTObjects_;
                    isomu20_eta2p1_vstar_    = struct_isomu20_eta2p1_vstar.nHLTObjects_;
                    isomu24_eta2p1_vstar_    = struct_isomu24_eta2p1_vstar.nHLTObjects_;
                    isomu30_eta2p1_vstar_    = struct_isomu30_eta2p1_vstar.nHLTObjects_;

                    mu5_version_             = struct_mu5_vstar.version_;
                    mu8_version_             = struct_mu8_vstar.version_;
                    mu12_version_            = struct_mu12_vstar.version_;
                    mu17_version_            = struct_mu17_vstar.version_;
                    mu15_eta2p1_version_     = struct_mu15_eta2p1_vstar.version_;
                    mu24_eta2p1_version_     = struct_mu24_eta2p1_vstar.version_;
                    mu30_eta2p1_version_     = struct_mu30_eta2p1_vstar.version_;
                    isomu20_eta2p1_version_  = struct_isomu20_eta2p1_vstar.version_;
                    isomu24_eta2p1_version_  = struct_isomu24_eta2p1_vstar.version_;
                    isomu30_eta2p1_version_  = struct_isomu30_eta2p1_vstar.version_;

                    dr_mu5_vstar_            = struct_mu5_vstar.dR_;
                    dr_mu8_vstar_            = struct_mu8_vstar.dR_;
                    dr_mu12_vstar_           = struct_mu12_vstar.dR_;
                    dr_mu17_vstar_           = struct_mu17_vstar.dR_;
                    dr_mu15_eta2p1_vstar_    = struct_mu15_eta2p1_vstar.dR_;
                    dr_mu24_eta2p1_vstar_    = struct_mu24_eta2p1_vstar.dR_;
                    dr_mu30_eta2p1_vstar_    = struct_mu30_eta2p1_vstar.dR_;
                    dr_isomu20_eta2p1_vstar_ = struct_isomu20_eta2p1_vstar.dR_;
                    dr_isomu24_eta2p1_vstar_ = struct_isomu24_eta2p1_vstar.dR_;
                    dr_isomu30_eta2p1_vstar_ = struct_isomu30_eta2p1_vstar.dR_;

                    hltps_mu5_vstar_            = struct_mu5_vstar.hltps_;
                    hltps_mu8_vstar_            = struct_mu8_vstar.hltps_;
                    hltps_mu12_vstar_           = struct_mu12_vstar.hltps_;
                    hltps_mu17_vstar_           = struct_mu17_vstar.hltps_;
                    hltps_mu15_eta2p1_vstar_    = struct_mu15_eta2p1_vstar.hltps_;
                    hltps_mu24_eta2p1_vstar_    = struct_mu24_eta2p1_vstar.hltps_;
                    hltps_mu30_eta2p1_vstar_    = struct_mu30_eta2p1_vstar.hltps_;
                    hltps_isomu20_eta2p1_vstar_ = struct_isomu20_eta2p1_vstar.hltps_;
                    hltps_isomu24_eta2p1_vstar_ = struct_isomu24_eta2p1_vstar.hltps_;
                    hltps_isomu30_eta2p1_vstar_ = struct_isomu30_eta2p1_vstar.hltps_;

                    // These are hardcoded to the value in Dima's table:
                    // http://dmytro.web.cern.ch/dmytro/trigger/triggerEvolution_all.html 
                    l1ps_mu5_vstar_            = L1_prescale("L1_SingleMu3"         );
                    l1ps_mu8_vstar_            = L1_prescale("L1_SingleMu3"         );
                    l1ps_mu12_vstar_           = L1_prescale("L1_SingleMu7"         );
                    l1ps_mu17_vstar_           = L1_prescale("L1_SingleMu12"        );
                    l1ps_mu15_eta2p1_vstar_    = L1_prescale("L1_SingleMu7"         );
                    l1ps_mu24_eta2p1_vstar_    = L1_prescale("L1_SingleMu16_Eta2p1" );
                    l1ps_mu30_eta2p1_vstar_    = L1_prescale("L1_SingleMu16_Eta2p1" );
                    l1ps_isomu20_eta2p1_vstar_ = L1_prescale("L1_SingleMu16_Eta2p1" );
                    l1ps_isomu24_eta2p1_vstar_ = L1_prescale("L1_SingleMu16_Eta2p1" );
                    l1ps_isomu30_eta2p1_vstar_ = L1_prescale("L1_SingleMu16_Eta2p1" );

                    ///////////////////////  
                    // End 2012 Triggers //
                    ///////////////////////

                    ///////////////////////  
                    // 2011 Triggers     //
                    ///////////////////////

                    // Muons
                    triggerMatchStruct struct_mu3_vstar       = MatchTriggerClass( mus_p4().at(iLep), &mu3_regexp      , 13);
                    triggerMatchStruct struct_mu15_vstar      = MatchTriggerClass( mus_p4().at(iLep), &mu15_regexp     , 13);
                    triggerMatchStruct struct_mu20_vstar      = MatchTriggerClass( mus_p4().at(iLep), &mu20_regexp     , 13);
                    triggerMatchStruct struct_mu24_vstar      = MatchTriggerClass( mus_p4().at(iLep), &mu24_regexp     , 13);
                    triggerMatchStruct struct_mu30_vstar      = MatchTriggerClass( mus_p4().at(iLep), &mu30_regexp     , 13);
                    triggerMatchStruct struct_mu8_Jet40_vstar = MatchTriggerClass( mus_p4().at(iLep), &mu8_Jet40_regexp, 13);

                    mu3_vstar_          = struct_mu3_vstar.nHLTObjects_;
                    mu15_vstar_         = struct_mu15_vstar.nHLTObjects_;
                    mu20_vstar_         = struct_mu20_vstar.nHLTObjects_;
                    mu24_vstar_         = struct_mu24_vstar.nHLTObjects_;
                    mu30_vstar_         = struct_mu30_vstar.nHLTObjects_;
                    mu8_Jet40_vstar_    = struct_mu8_Jet40_vstar.nHLTObjects_;

                    mu3_version_        = struct_mu3_vstar.version_;
                    mu15_version_       = struct_mu15_vstar.version_;
                    mu20_version_       = struct_mu20_vstar.version_;
                    mu24_version_       = struct_mu24_vstar.version_;
                    mu30_version_       = struct_mu30_vstar.version_;
                    mu8_Jet40_version_  = struct_mu8_Jet40_vstar.version_;

                    dr_mu3_vstar_       = struct_mu3_vstar.dR_;
                    dr_mu15_vstar_      = struct_mu15_vstar.dR_;
                    dr_mu20_vstar_      = struct_mu20_vstar.dR_;
                    dr_mu24_vstar_      = struct_mu24_vstar.dR_;
                    dr_mu30_vstar_      = struct_mu30_vstar.dR_;
                    dr_mu8_Jet40_vstar_ = struct_mu8_Jet40_vstar.dR_;

                    hltps_mu3_vstar_       = struct_mu3_vstar.hltps_;
                    hltps_mu15_vstar_      = struct_mu15_vstar.hltps_;
                    hltps_mu20_vstar_      = struct_mu20_vstar.hltps_;
                    hltps_mu24_vstar_      = struct_mu24_vstar.hltps_;
                    hltps_mu30_vstar_      = struct_mu30_vstar.hltps_;
                    hltps_mu8_Jet40_vstar_ = struct_mu8_Jet40_vstar.hltps_;

                    ///////////////////////  
                    // End 2011 Triggers //
                    ///////////////////////

                    //////////////
                    // Jets     //
                    //////////////

                    // Calo Jets
                    // Find the highest Pt jet separated by at least dRcut from this lepton and fill the jet Pt
                    ptj1_       = -999.0;
                    ptj1_b2b_   = -999.0;
                    dphij1_b2b_ = -999.0;
                    nj1_        = 0;
                    for (unsigned int iJet = 0; iJet < jets_p4().size(); iJet++) {
                        double dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), jets_p4().at(iJet) );
                        if( dr > deltaRCut && jets_p4().at(iJet).pt() > 10 ) nj1_++;
                        if ( dr > deltaRCut && jets_p4().at(iJet).pt() > ptj1_ ){
                            ptj1_ = jets_p4().at(iJet).pt();
        
                            // back to back in phi
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( mus_p4().at(iLep), jets_p4().at(iJet) ) );
                            if( dphi > deltaPhiCut && jets_p4().at(iJet).pt() > ptj1_b2b_ ){        
                                ptj1_b2b_   = jets_p4().at(iJet).pt();
                                dphij1_b2b_ = dphi;
                            }
                        }
                    }
  
                    // PF Jets
                    // Find the highest Pt pfjet separated by at least dRcut from this lepton and fill the pfjet Pt
                    ptpfj1_       = -999.0;
                    ptpfj1_b2b_   = -999.0;
                    dphipfj1_b2b_ = -999.0;
                    npfj1_        = 0;
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        double dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), pfjets_p4().at(iJet) );
                        if( dr > deltaRCut && pfjets_p4().at(iJet).pt() > 10 ) npfj1_++;
                        if ( dr > deltaRCut && pfjets_p4().at(iJet).pt() > ptpfj1_ ){
                            ptpfj1_ = pfjets_p4().at(iJet).pt();
        
                            // back to back in phi
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( mus_p4().at(iLep), pfjets_p4().at(iJet) ) );
                            if( dphi > deltaPhiCut && pfjets_p4().at(iJet).pt() > ptpfj1_b2b_ ){        
                                ptpfj1_b2b_   = pfjets_p4().at(iJet).pt();
                                dphipfj1_b2b_ = dphi;
                            }
                        }
                    }
  
                    // L2L3 PF Jets
                    // Find the highest Pt PF corrected jet separated by at least dRcut from this lepton and fill the jet Pt
                    ptpfcj1_       = -999.0;
                    ptpfcj1_b2b_   = -999.0;
                    dphipfcj1_b2b_ = -999.0;
                    npfcj1_        = 0;
                    btagpfc_       = false;
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        // JetID
                        if ( !passesPFJetID(iJet)) continue;
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = jetCorrection(jp4, jet_pf_L2L3corrector);
                        LorentzVector jp4cor = jp4 * jet_cor;
                        if (jp4cor.pt() > 15 && pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) > 1.74 ) btagpfc_ = true;
                        double dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), jp4cor );
                        if( dr > deltaRCut && jp4cor.pt() > 10 ) npfcj1_++;
                        if ( dr > deltaRCut && jp4cor.pt() > ptpfcj1_ ){
                            ptpfcj1_ = jp4cor.pt();
    
                            // back to back in phi
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( mus_p4().at(iLep), jp4cor ) );
                            if( dphi > deltaPhiCut && jp4cor.pt() > ptpfcj1_b2b_ ){
                                ptpfcj1_b2b_   = jp4cor.pt();
                                dphipfcj1_b2b_ = dphi;
                            }
                        }
                    }

                    // L1FastL2L3 PF Jets
                    // Find the highest Pt PF corrected jet separated by at least dRcut from this lepton and fill the jet Pt
                    emfpfcL1Fj1_      = -999.0;
                    ptpfcL1Fj1_       = -999.0;
                    dphipfcL1Fj1_ 	  = -999.0;
                    ptpfcL1Fj1_b2b_   = -999.0;
                    dphipfcL1Fj1_b2b_ = -999.0;
                    npfcL1Fj1_        = 0;
                    npfc30L1Fj1_      = 0;
                    npfc40L1Fj1_      = 0;
                    btagpfcL1F_       = false;
                    rho_ = cms2.evt_rho();
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        // JetID
                        if ( !passesPFJetID(iJet)) continue;
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = cms2.pfjets_corL1FastL2L3()[iJet];
                        LorentzVector jp4cor = jp4 * jet_cor;
                        if (jp4cor.pt() > 15 && pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) > 1.74 ) btagpfcL1F_ = true;
                        double dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), jp4cor );
                        if( dr > deltaRCut && jp4cor.pt() > 10 ) npfcL1Fj1_++;
                        if( dr > deltaRCut && jp4cor.pt() > 30 ) npfc30L1Fj1_++;
                        if( dr > deltaRCut && jp4cor.pt() > 40 ) npfc40L1Fj1_++;
                        if ( dr > deltaRCut && jp4cor.pt() > ptpfcL1Fj1_ ){
                            emfpfcL1Fj1_ = (cms2.pfjets_chargedEmE().at(iJet) + cms2.pfjets_neutralEmE().at(iJet)) / pfjets_p4().at(iJet).E();
                            ptpfcL1Fj1_ = jp4cor.pt();
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( mus_p4().at(iLep), jp4cor ) );
                            dphipfcL1Fj1_ = dphi;

                            // back to back in phi
                            if( dphi > deltaPhiCut && jp4cor.pt() > ptpfcL1Fj1_b2b_ ){
                                ptpfcL1Fj1_b2b_   = jp4cor.pt();
                                dphipfcL1Fj1_b2b_ = dphi;
                            }
                        }
                    }

					//***  Doing B-tagging correctly ***
                    // B-tagged L1FastL2L3 PF Jets
                    // Find the highest Pt B-tagged PF L1FastL2L3 corrected jet separated by at least dRcut from this lepton and fill the jet Pt
                    ptbtagpfcL1Fj1_       = -999.0;
                    dphibtagpfcL1Fj1_       = -999.0;
                    for (unsigned int iJet = 0; iJet < pfjets_p4().size(); iJet++) {
                        if ( !passesPFJetID(iJet)) continue;
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = cms2.pfjets_corL1FastL2L3()[iJet];
                        LorentzVector jp4cor = jp4 * jet_cor;
                        if (pfjets_simpleSecondaryVertexHighEffBJetTag().at(iJet) < 1.74 ) continue;
                        double dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), jp4cor );
                        if ( dr > deltaRCut && jp4cor.pt() > ptbtagpfcL1Fj1_ ){
                            ptbtagpfcL1Fj1_ = jp4cor.pt();
                            float dphi = fabs( ROOT::Math::VectorUtil::DeltaPhi( mus_p4().at(iLep), jp4cor ) );
							dphibtagpfcL1Fj1_ = dphi; 
                        }
                    }

                    //////////////
                    // End Jets //
                    //////////////

                    ///////////////////
                    // B Tagging     //
                    ///////////////////

                    // The btag information
                    nbjet_ = this_nbjet;
                    dRbNear_ =  99.;
                    dRbFar_  = -99.;
                    for (int ii=0; ii<nbjet_; ii++) {
                        unsigned int iJet = bindex[ii];
                        float dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), jets_p4().at(iJet));
                        if (dr < dRbNear_) dRbNear_ = dr;
                        if (dr > dRbFar_)  dRbFar_  = dr;
                    }
 
                    // The btag information for pfjets
                    nbpfcjet_ = this_nbpfjet;
                    dRbpfcNear_ = 99.;
                    dRbpfcFar_  = -99.;
                    for (int ii=0; ii<nbpfcjet_; ii++) {
                        unsigned int iJet = bpfindex[ii];
                        LorentzVector jp4 = pfjets_p4()[iJet];
                        float jet_cor = jetCorrection(jp4, jet_pf_L2L3corrector);
                        LorentzVector jp4cor = jp4 * jet_cor;
                        float dr = ROOT::Math::VectorUtil::DeltaR( mus_p4().at(iLep), jp4cor);
                        if (dr < dRbpfcNear_) dRbpfcNear_ = dr;
                        if (dr > dRbpfcFar_)   dRbpfcFar_  = dr;
                    }

                    ///////////////////
                    // End B Tagging //
                    ///////////////////


                    // Time to fill the baby for the muons
                    FillBabyNtuple();
      
                }// closes loop over muons
            } // closes if statements about whether we want to fill muons

        }// closes loop over events
        //printf("Good events found: %d out of %d\n",nGoodEvents,nEntries);

    }  // closes loop over files

    bmark.Stop("benchmark");
    cout << endl;
    cout << nEventsTotal << " Events Processed" << endl;
    //cout << "# of bad events filtered = " << bad_events << endl; 
    //cout << "# of duplicates filtered = " << duplicates << endl; 
    cout << "------------------------------" << endl;
    cout << "CPU  Time:	" << Form("%.01f", bmark.GetCpuTime("benchmark" )) << endl;
    cout << "Real Time:	" << Form("%.01f", bmark.GetRealTime("benchmark")) << endl;
    cout << endl;
    
    CloseBabyNtuple();
    return;
    
} // closes myLooper function  

