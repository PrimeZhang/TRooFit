/*****************************************************************************
 * Project: TRooFit   - an extension for RooFit                              *
 *                                                                           *
 * Modified version of a 
 * RooWorkspace ... Helps with model building                          * 
 *****************************************************************************/

#ifndef TROOWORKSPACE
#define TROOWORKSPACE

//next lines to hack RooFitResult so that statusHistory can be presered when fitTo
#define protected public
#include "RooFitResult.h"
#undef protected

#include "RooWorkspace.h"

#include "TRooFit/TRooHStack.h"
#include "TRooFit/TRooH1.h"
#include "TRooFit/TRooHF1.h"

#include "TTree.h"
#include "TLegend.h"

#include "RooSimultaneous.h"
 
class TRooWorkspace : public RooWorkspace {
public:
  //using RooWorkspace::RooWorkspace; 
  
  TRooWorkspace() : RooWorkspace() { }
  TRooWorkspace(const char* name, const char* title = 0) : RooWorkspace(name,title) { }
  TRooWorkspace(const RooWorkspace& other);
  ~TRooWorkspace() { fNll.removeAll(); }
  

  bool definePoi(const char* poi) { return defineSet("poi",poi); }
  bool addArgument(const char* name, const char* title, double val);
  RooRealVar* addParameter(const char* name, const char* title, double val, double min, double max, const char* constraintType=0);
  RooRealVar* addParameter(const char* name, const char* title, double min, double max, const char* constraintType=0);
  RooRealVar* addObservable(const char* name, const char* title, double min, double max);
  RooRealVar* addObservable(const char* name, const char* title, int nBins, double min, double max);
  RooRealVar* addObservable(const char* name, const char* title, int nBins, const double* bins);
  TRooHStack* addChannel(const char* name, const char* title, const char* observable, int nBins, double min, double max);
  TRooHStack* addChannel(const char* name, const char* title, const char* observable, int nBins, const double* bins);
  TRooHStack* addChannel(const char* name, const char* title, const char* observable);
  bool addSample(const char* name, const char* title, const char* channels="*", bool allowNegative=false);
  
  TRooHF1* addFactor(const char* name, const char* title, double nomVal=1.);
  bool factorSetVariation(const char* name, const char* parName, double parVal, double val);
  
  bool dataFill(const char* channel, double x, double w=1.);
  Int_t sampleFill(const char* sample, const char* channel, double x, double w=1.);
  bool sampleAdd(const char* sample, const char* channel,  TH1* h1);
  bool sampleAdd(const char* sample, const char* channel, RooAbsReal& arg);
  bool sampleAddVariation(const char* sample, const char* channel, const char* parName, double parVal, TH1* h1);
  bool sampleFill(const char* sample, TTree* tree, const char* weight, const char* variationName=0, double variationVal=0); //fills given sample in all channels where formula have been defined
  
  void SetBinContent(const char* sampleName, const char* channelName, Int_t bin, double val) { sample(sampleName,channelName)->SetBinContent(bin,val); }
  void SetVariationBinContent(const char* sample, const char* channel, const char* parName, double parVal, Int_t bin, double val);
  
  //add a normalization factor to a sample, across all channels
  void sampleScale(const char* sample,RooAbsReal& arg);
  void sampleScale(const char* sampleName,const char* par) { if(var(par)) sampleScale(sampleName,*var(par)); }
  
  //set fill color of sample in all channels
  void sampleSetFillColor(const char* sample, Int_t in);
  void sampleSetLineColor(const char* sample, Int_t in);
  
  double sampleIntegralAndError(double& err, const char* sampleFullName, const TRooFitResult& res="") const;
  double sampleIntegralAndError(double& err, const char* channelName, unsigned int sampleNumber, const TRooFitResult& fr="") const;
  
  TRooH1* sample(const char* sampleName, const char* channelName);
  TRooAbsHStack* channel(const char* name) const;
  TRooHF1* factor(const char* factorName);
  
  void setData(const char* dataName) { 
    if(!data(dataName)) return;
    fCurrentData = dataName; 
  }
  
  void DisableForcedRecommendedOption(bool in) { kDisabledForcedRecommendedOptions=in; } //use to override forcing of the recommended fit options when calling fitTo
  
  double impact(const char* poi, const char* np, bool positive=true);
  void impact(const char* impactPar=0, float correlationThreshold=0);
  
  RooFitResult* fitTo(RooAbsData* theData, const RooArgSet* globalObserables=0, bool doHesse=true);
  RooFitResult* fitTo(const char* dataName=0, bool doHesse=true, const RooArgSet& minosPars=RooArgSet());
  RooFitResult* loadFit(const char* fitName,bool prefit=false);
  RooFitResult* getFit(const char* fitName=0) { return dynamic_cast<RooFitResult*>(obj((fitName==0)?fCurrentFit.Data():fitName)); }
  RooAbsReal* getFitNll(const char* fitName=0);
  
  RooAbsReal* nll(const char* nllName) const { return dynamic_cast<RooAbsReal*>(fNll.find(nllName)); }
  
  double pll(RooAbsData* theData, const RooArgSet* globalObservables=0, bool oneSided=false, bool discovery=false);
  
  void addLabel(const char* label) { fLabels.push_back(label); }
  
  TLegend* GetLegend();
  
  double GetSampleCoefficient(const char* sampleFullName) const;
  
  RooSimultaneous* model(const char* channels="*");
  
  bool generateAsimov(const char* name, const char* title, bool fitToObsData=true);
  std::pair<RooAbsData*,RooArgSet*> generateToy(const char* name, const char* title, bool fitToObsData=true);
  
  
  //controls which channels are visible when drawing things
  //sets the 'hidden' attribute on non-visible channels
  Int_t SetVisibleChannels(const char* filter) { 
    setChannelAttribute("*","hidden",kTRUE); //hide all channels first
    return setChannelAttribute(filter,"hidden",kFALSE); //unhide the selected ones
  }
  Int_t setChannelAttribute(const char* channels,const char* attribute,Bool_t val=kTRUE);
  Int_t setVarAttribute(const char* vars,const char* attribute,Bool_t val=kTRUE);
  
  //draw a channel's stack and overlay the data too
  void channelDraw(const char* channel, Option_t* option="e3005", const TRooFitResult& res = "");
  
  //draws all channels
  virtual void Draw(Option_t* option, const TRooFitResult& res);
  virtual void Draw(Option_t* option="e3005") { 
    //if(fCurrentFit!="") Draw(option,getFit(fCurrentFit));
    if(fCurrentFitResult) Draw(option,*fCurrentFitResult);
    else Draw(option,""); 
  }
  
  //draws all channels, showing how values of channels depend on var
  void DrawDependence(const char* var, Option_t* option="TRI1");
  
  void SetRatioHeight(double in, bool showSignificance=false) { 
    if(fLegend) fLegend->SetTextSize( fLegend->GetTextSize() * (1. - fRatioHeight) / (1. - in) );
    fRatioHeight = in; 
    kShowSignificance=showSignificance;
  }
  
  Bool_t writeToFile(const char* fileName, Bool_t recreate=kTRUE);
  
  virtual void Print(Option_t* opt="") const;
  
  static void setDefaultStyle();
  
private:
  TString fChannelCatName = "channelCat"; //should only change if wrapping a non-conventional workspace
  TString fSimPdfName = "simPdf"; //should only change if wrapping a non-conventional workspace

  std::map<TString,TH1*> fDummyHists;
  
  TString fCurrentData = "obsData";
  
  TString fCurrentFit = "";
  Bool_t fCurrentFitIsPrefit = false; 
  TRooFitResult* fCurrentFitResult = 0;
  
  RooArgList fStagedChannels; //channels cannot be added until they are frozen (all content filled)
  
  TLegend* fLegend = 0;

  std::vector<TString> fLabels; //plot labels

  Double_t fRatioHeight = 0; //if nonzero, channelDraw will draw ratio plots

  Bool_t fIsHFWorkspace = false; //if true, this is a histfactory workspace
  Bool_t kDisabledForcedRecommendedOptions = false;
  
  Bool_t kShowSignificance = false;
  
  RooArgList fNll; //!
  
  ClassDef(TRooWorkspace,1) // An extended form of a RooWorkspace
};
 
#endif
