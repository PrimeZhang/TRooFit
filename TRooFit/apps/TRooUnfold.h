
#ifndef TROOUNFOLD
#define TROOUNFOLD

#include "TRooFit/TRooHStack.h"
#include "TRooFit/TRooH1D.h"
#include "TRooFit/TRooHF1D.h"
#include "TRooFit/TRooGPConstraint.h"
#include "TNamed.h"
#include "RooDataSet.h"


class TRooUnfold : public TNamed  {

  public:
    
    
    TRooUnfold() : TNamed() { }
    TRooUnfold(const char* name, const char* title,int nBinsReco,double recoLow,double recoHigh,int nBinsTruth,double truthLow,double truthHigh);
    TRooUnfold(const char* name, const char* title,TH2* nominalMigrationMatrix);
    
    RooRealVar* GetRecoVar() { return m_recoVar; }
    RooRealVar* GetTruthVar() { return m_truthVar; }
    
    Bool_t AddMigrationMatrix(TH2* matrix, const char* variation="Nominal"); //required
    Bool_t AddTruthSignal(TH1* truth, const char* variation="Nominal"); //triggers inefficiency
    Bool_t AddRecoSignal(TH1* reco, const char* variation="Nominal"); //triggers impurity (out-of-fiducial contributions)
    Bool_t AddBackground(TH1* reco, const char* bkgName, const char* variation="Nominal"); //triggers additional components
    Bool_t AddScaleFactor(TH1* reco, const char* sfName, const char* variation="Nominal"); //does nothing until ApplyFactorToComponent is called
    Bool_t AddNormFactor(const char* sfName, const char* sfTitle=0);
    Bool_t AddData(TH1* data);
    
    Bool_t AddRegularization(TH1* hist);
    
    Bool_t ApplyFactorToComponent(const char* sfName, const char* compName); //special case of compName=signal will apply to all signal comps
    
    Bool_t ClearSignalMCUncert(); //call this to remove all signal MC uncert from the setup
    
    ClassDef(TRooUnfold,1);

    const RooArgSet& poi() { return m_poi; }

    void SetPrintLevel(RooFit::MsgLevel level) { m_printLevel = level; } //use RooFit::INFO etc etc

    TRooHF1D* GetScaleFactor(const char* sfName) {
      if(m_sfFunctions.find(sfName)==m_sfFunctions.end()) return 0;
      return m_sfFunctions[sfName];
    }

    RooRealVar* GetSigNormVar(int bin);

    Bool_t BuildModel(); //Finishes constructing the TRooFit model

    TH1* GetFiducialPurity(const char* variation="Nominal") {
      if(!m_builtModel) return 0;
      return m_fidPurity[variation];
    }
    
    TH1* GetEfficiency(const char* variation="Nominal") {
      if(!m_builtModel) return 0;
      return m_efficiency[variation];
    }
    
    TRooHStack* GetStack() { return m_stack; }

    TRooH1D* GetComponent(const char* name) { 
      if(m_bkgPdfs.find(name)==m_bkgPdfs.end()) return 0;
      return m_bkgPdfs[name];
    }

    TH1* GetData() { return m_data; }

    TH1* GetRegularizationHistogram() { return m_regularizationHist; }

    TRooH1D* GetTruth() { return m_result; }
    RooFitResult* GetFitResult() { return m_fitResult; }
    
    TH1* GetInputHistogram(const char* comp, const char* variation="Nominal") {
      if(m_bkg.find(comp)==m_bkg.end()) return 0;
      if(m_bkg[comp].find(variation)==m_bkg[comp].end()) return 0;
      return m_bkg[comp][variation];
    }
    
    TH1* GetPrefitTruthHistogram();
    std::vector<TH1*> GetPostfitTruthHistograms(const std::vector<TString>&& systGroups={}, bool doSTATCORR=false);
    
    RooAbsArg* GetServer(const char* name) {
      if(!m_fullModel) BuildModel();
      RooArgSet s; m_fullModel->treeNodeServerList(&s);
      return dynamic_cast<RooAbsArg*>(s.find(name));
    }
    
    TH1* GetSignificanceHistogram();
    TH1* GetExpectedRecoHistogram(RooFitResult* fr=0);
    TH1* GetResidualHistogram(RooFitResult* fr=0);
    
    TRooGPConstraint* GetRegularizationConstraint() { return m_regularizationConstraint; }
    
    RooFitResult* Fit(TH1* data = 0, bool doMinos=true);
    
    virtual void Print(Option_t* opt="") const;
    
    void SetFloor(double floor=1e-9) { m_floor=floor; } //set to negative value to remove the floor
    
    
    
    RooAbsData* m_dataSet = 0;
    RooAbsPdf* m_fullModel = 0;
    RooAbsReal* m_nll = 0;
    
    Double_t ScanRegularizationStrength(TGraph** g = 0);
    
    Double_t GetNLLMin();
    Double_t GetNLLMax();
    Double_t GetConstraintNLLMax();
    
    Double_t GetNLL();
    Double_t GetConstraintNLL();
    
    
    void WriteModelInputs(const char* file, const char* opt="RECREATE");
    void WriteHistograms(const char* file,const char* opt="RECREATE");
    
  private:
    bool m_builtModel = false;
    
    RooRealVar* m_recoVar;
    RooRealVar* m_truthVar;
    RooArgSet m_npAndUnconstrained;
    RooArgSet m_poi; //the sigNorm parameters
  
    RooFit::MsgLevel m_printLevel = RooFit::ERROR;
    
    std::map<TString,TH2*> m_responseMatrix; //normalized migration matrices
    std::map<TString,TH1*> m_fidPurity; //fraction of signal at reco level (in each bin) that had a corresponding truth level value (i.e. came from fiducial volume)
    std::map<TString,TH1*> m_efficiency; //fraction of signal at truth level (in each bin) that has a corresponding reco level value (i.e. is reconstructed)
    
    std::map<TString,TH2*> m_migrationMatrix;
    std::map<TString,TH1*> m_truthSignal;
    std::map<TString,TH1*> m_recoSignal;
    std::map<TString,std::map<TString,TH1*>> m_bkg; //first index is bkg name, second is variation
    std::vector<TString> m_bkgOrder;
    std::map<TString,std::map<TString,TH1*>> m_recoSF;
    std::map<TString,RooRealVar*> m_normFactor;
    
    std::map<TString,std::set<TString>> m_sfApplied; //index is sf name, set is list of components that the sf is applied to
    
    TH1* m_data = 0;

    std::map<TString,TRooHF1D*> m_sfFunctions;
    std::map<TString,TRooH1D*> m_bkgPdfs;
    std::map<TString,RooRealVar*> m_systNP;
    
    std::map<TString,TString> m_systGroup; //map from nuisance parameter name to systematic group
    
    TRooHStack* m_stack = 0;

    TH1* m_regularizationHist = 0;
    
    TRooH1D* m_result = 0;

    RooFitResult* m_fitResult = 0;
    
    std::map<TString,RooFitResult*> m_fitResults;

    TRooGPConstraint* m_regularizationConstraint = 0;

    RooRealVar* m_regStrength = 0; //regularization strength


    TH1* m_prefitTruth = 0;
    
    double m_floor = 1e-9;
    
    double m_constraintMax=0;
    double m_nllMin = 0;
    double m_nllMax = 0;
    
};

#endif