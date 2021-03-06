#include "ChainFromText.cc"

#include "TChain.h"

void runFR(){

gROOT->LoadMacro("myBabyMaker.C++");

//////////
// 2011 //
//////////

  // Data

    // Double Electron
    TChain *chain1 = new TChain("Events");
    chain1->Add("/hadoop/cms/store/user/yanjuntu/CMSSW_4_2_4_V04-02-20/DoubleElectron_Run2011A-PromptReco-v4_AOD/CMSSW_4_2_4_V04-02-20_merged/V04-02-20/*.root");
    //chain1->Add("/hadoop/cms/store/user/yanjuntu/CMSSW_4_2_4_V04-02-20/DoubleElectron_Run2011A-PromptReco-v4_AOD/CMSSW_4_2_4_V04-02-20_merged/V04-02-20/*.root");
    //chain1->Add("/hadoop/cms/store/user/yanjuntu/CMSSW_4_2_7_patch1_V04-02-30/DoubleElectron_Run2011A-05Aug2011-v1_AOD/CMSSW_4_2_7_patch1_V04-02-30_merged/V04-02-30/*.root"); 
    //chain1->Add("/hadoop/cms/store/user/yanjuntu/CMSSW_4_2_7_patch1_V04-02-30/DoubleElectron_Run2011A-PromptReco-v6_AOD/CMSSW_4_2_7_patch1_V04-02-30_merged/V04-02-30/*.root"); 
    myBabyMaker* baby1 = new myBabyMaker();
    baby1->ScanChain(chain1, "DoubleElectron_10Sep2011.root", true, -1);
    return;

/*
    // Single Muon
    TChain *chain3 = new TChain("Events");
    //chain3->Add("/hadoop/cms/store/user/jaehyeok/CMSSW_4_1_2_patch1_V04-01-03/SingleMu_Run2011A-PromptReco-v2_AOD/CMSSW_4_1_2_patch1_V04-01-03_merged/V04-01-03/*.root");
    //chain3->Add("/hadoop/cms/store/user/jaehyeok/CMSSW_4_1_2_patch1_V04-00-13/SingleMu_Run2011A-PromptReco-v1_AOD/CMSSW_4_1_2_patch1_V04-00-13_merged/V04-00-13/*.root");
    myBabyMaker* baby3 = new myBabyMaker();
    baby3->ScanChain(chain3, "SingleMu_10Sep2011.root", true, -1);

    // Double Muon
    TChain *chain2 = new TChain("Events");
    //chain2->Add("/hadoop/cms/store/user/yanjuntu/CMSSW_4_1_2_patch1_V04-01-03/DoubleMu_Run2011A-PromptReco-v2_AOD/CMSSW_4_1_2_patch1_V04-01-03_merged/V04-01-03/*.root")
    //chain2->Add("/hadoop/cms/store/user/yanjuntu/CMSSW_4_1_2_patch1_V04-00-13/DoubleMu_Run2011A-PromptReco-v1_AOD/CMSSW_4_1_2_patch1_V04-00-13_merged/V04-00-13/*.root");
    myBabyMaker* baby2 = new myBabyMaker();
    baby2->ScanChain(chain2, "DoubleMu_10Sep2011.root", true, -1);
    return;
*/

  // MC

// Monte Carlo

  // QCD 30to50
  TChain *chain4 = ChainFromText( "input_data/qcd_pt_30to50_fall10_uaf.txt" );
  myBabyMaker * baby4 = new myBabyMaker();
  baby4->ScanChain(chain4, "qcd_pt_30to50_fall10.root", false, -1);

  // QCD 50to80
  TChain *chain5 = ChainFromText( "input_data/qcd_pt_50to80_fall10_uaf.txt" );
  myBabyMaker * baby5 = new myBabyMaker();
  baby5->ScanChain(chain5, "qcd_pt_50to80_fall10.root", false, -1);

  // QCD 80to120
  TChain *chain6 = ChainFromText( "input_data/qcd_pt_80to120_fall10_uaf.txt" );
  myBabyMaker * baby6 = new myBabyMaker();
  baby6->ScanChain(chain6, "qcd_pt_80to120_fall10.root", false, -1);

  /*
  // QCD 30
  TChain *chain4 = ChainFromText( "input_data/qcd30_uaf.txt" );
  myBabyMaker * baby4 = new myBabyMaker();
  baby4->ScanChain(chain4, "qcd30.root", false, -1);

  // QCD 50
  TChain *chain5 = ChainFromText( "input_data/qcd50_uaf.txt" );
  myBabyMaker * baby5 = new myBabyMaker();
  baby5->ScanChain(chain5, "qcd50.root", false, -1);

  // QCD 80
  TChain *chain6 = ChainFromText( "input_data/qcd80_uaf.txt" );
  myBabyMaker * baby6 = new myBabyMaker();
  baby6->ScanChain(chain6, "qcd80.root", false, -1);
  */

  // MuEnriched 10
  TChain *chain7 = ChainFromText( "input_data/mu10_uaf.txt" );
  myBabyMaker * baby7 = new myBabyMaker();
  baby7->ScanChain(chain7, "mu10.root", false, -1);

  // MuEnriched 15
  TChain *chain8 = ChainFromText( "input_data/mu15_uaf.txt" );
  myBabyMaker * baby8 = new myBabyMaker();
  baby8->ScanChain(chain8, "mu15.root", false, -1);

  // Wmunu
//  TChain *chain6 = ChainFromText( "input_data/wmunu_uaf.txt" );
//  myBabyMaker * baby6 = new myBabyMaker();
//  baby6->ScanChain(chain6, "Wmunu.root", false, -1);

  // Wenu
//  TChain *chain7 = ChainFromText( "input_data/wenu_uaf.txt" );
//  myBabyMaker * baby7 = new myBabyMaker();
//  baby7->ScanChain(chain7, "Wenu.root", false, -1);

  // InclusiveMu 15
//  TChain *chain8 = ChainFromText( "input_data/inclMu_uaf.txt" );
//  myBabyMaker * baby8 = new myBabyMaker();
//  baby8->ScanChain(chain8, "inclMu.root", false, -1);

}
