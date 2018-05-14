#include "TRooFit/TRooWorkspace.h"

#include "TRooFit/TRooHStack.h"
#include "TRooFit/TRooH1D.h"

#include "RooCategory.h"
#include "RooDataSet.h"

bool TRooWorkspace::addParameter(const char* name, const char* title, double min, double max, const char* constraintType) {
  factory(Form("%s[%f,%f]",name,min,max));
  RooRealVar* v = var(name);
  if(!v) return false;
  v->SetTitle(title);
  
  if(constraintType) v->setStringAttribute("constraintType",constraintType);
  
  return true;
}

bool TRooWorkspace::addObservable(const char* name, const char* title, double min, double max) {
  if(!addParameter(name,title,min,max)) return false;
  
  if(!set("obs")) defineSet("obs",name);
  else extendSet("obs",name);
  
  return true;
}

bool TRooWorkspace::addArgument(const char* name, const char* title, double val) {
  factory(Form("%s[%f]",name,val));
  RooRealVar* v = var(name);
  if(!v) return false;
  v->SetTitle(title);
  v->setConstant(true);
  return true;
}

bool TRooWorkspace::addChannel(const char* name, const char* title, const char* observable, int nBins, double min, double max) {
  RooCategory* c = cat("channelCat");
  if(!c) {
    factory(Form("channelCat[%s=0]",name));
    c = cat("channelCat");
    if(!set("obs")) defineSet("obs","channelCat");
    else extendSet("obs","channelCat");
    
    if(!c) return false;
  }
  else {
    c->defineType(name);
  }
  
  
  
  RooRealVar* v = var(observable);
  if(!v) return false;
  
  //create a TRooHStack and import it ..
  TRooHStack* hs = new TRooHStack(name,title);
  fStagedChannels.add(*hs);
  hs->SetMinimum(0);
  //import(*hs);
  
  //need to store a dummyHist for the binning ...
  fDummyHists[name] = new TH1D(observable,"Data",nBins,min,max);
  fDummyHists[name]->SetDirectory(0);
  
  return true;
}

bool TRooWorkspace::addChannel(const char* name, const char* title, const char* observable, int nBins, const double* bins) {
  RooCategory* c = cat("channelCat");
  if(!c) {
    factory(Form("channelCat[%s=0]",name));
    c = cat("channelCat");
    if(!set("obs")) defineSet("obs","channelCat");
    else extendSet("obs","channelCat");
    
    if(!c) return false;
  }
  else {
    c->defineType(name);
  }
  
  
  
  RooRealVar* v = var(observable);
  if(!v) return false;
  
  //create a TRooHStack and import it ..
  TRooHStack* hs = new TRooHStack(name,title);
  fStagedChannels.add(*hs);
  hs->SetMinimum(0);
  //import(*hs);
  
  //need to store a dummyHist for the binning ...
  fDummyHists[name] = new TH1D(observable,"Data",nBins,bins);
  fDummyHists[name]->SetDirectory(0);
  
  return true;
}

bool TRooWorkspace::addChannel(const char* name, const char* title, const char* observable) {
  RooRealVar* v = var(observable);
  if(!v) return false;
  if(v->getBinning(name).isUniform()) {
    return addChannel(name,title,observable,v->getBins(name),v->getMin(name),v->getMax(name));
  } else {
    return addChannel(name,title,observable,v->getBins(name),v->getBinning(name).array());
  }
}

#include "TRegexp.h"

bool TRooWorkspace::addSample(const char* name, const char* title, const char* channels) {
  
  TRegexp pattern(channels,true);
  
  //loop over channels, create sample for each one that matches pattern
  TIterator* catIter = cat("channelCat")->typeIterator();
  TObject* c;
  while( (c = catIter->Next()) ) {
    TString cName(c->GetName());
    if(cName.Contains(pattern)) {
      fDummyHists[c->GetName()]->Reset(); //ensures it is empty
      TString hName(Form("%s_%s",name,c->GetName()));
      TRooH1D* h = new TRooH1D(hName,title,*var(fDummyHists[c->GetName()]->GetName()),fDummyHists[c->GetName()]);
      //import(*h);
      channel(c->GetName())->Add(h);
    }
  }
  
  delete catIter;
  
  return true;
  
}
  
bool TRooWorkspace::dataFill(const char* channelName, double x, double w) {
  if(!data("obsData")) {
    RooArgSet s(*set("obs"));
    RooRealVar w("weightVar","weightVar",1);
    s.add(w);
    RooDataSet* data = new RooDataSet("obsData","obsData",s,"weightVar");
    import(*data);
  }
  
  cat("channelCat")->setLabel(channelName);
  var( fDummyHists[channelName]->GetName() )->setVal( x );
  
  data("obsData")->add(*set("obs"),w);
  
  return true;
  
}

#include "TCut.h"

bool TRooWorkspace::sampleFill(const char* sampleName, TTree* tree, const char* weight) {
  TString sWeight(weight);
  //loop over channels, create sample for each one that matches pattern
  TIterator* catIter = cat("channelCat")->typeIterator();
  TObject* c;
  while( (c = catIter->Next()) ) {
    TRooH1* s = sample(sampleName,c->GetName());
    TRooHStack* cc = channel(c->GetName());
    
    TH1* histToFill = (TH1*)fDummyHists[c->GetName()]->Clone("tmpHist");
    histToFill->Reset();
    tree->Draw(Form("%s>>tmpHist",var( fDummyHists[c->GetName()]->GetName() )->getStringAttribute("formula")), TCut("cut",cc->getStringAttribute("formula"))*sWeight);
    s->Add(histToFill);
    delete histToFill;
    
  }
  delete catIter;
  
  return true;
}

Int_t TRooWorkspace::sampleFill(const char* sampleName, const char* channelName,  double x, double w) {
  return sample(sampleName,channelName)->Fill(x,w);
}
bool TRooWorkspace::sampleAdd(const char* sampleName, const char* channelName,  TH1* h1) {
  return sample(sampleName,channelName)->Add(h1);
}
bool TRooWorkspace::sampleAdd(const char* sampleName, const char* channelName,  RooAbsReal& arg) {
  return sample(sampleName,channelName)->Add(arg);
}

TRooH1* TRooWorkspace::sample(const char* sampleName, const char* channelName) {
  RooAbsReal* chan = dynamic_cast<RooAbsReal*>(channel(channelName));
  TRooH1* out = dynamic_cast<TRooH1*>(chan->findServer(Form("%s_%s",sampleName,channelName)));
  return out;
}

TRooHStack* TRooWorkspace::channel(const char* name) {
  TRooHStack* out =  dynamic_cast<TRooHStack*>(pdf(name));
  if(!out) {
    out = dynamic_cast<TRooHStack*>(fStagedChannels.find(name));
  }
  return out;
}
  

//set fill color of sample in all channels
void TRooWorkspace::sampleSetFillColor(const char* sampleName, Int_t in) {
  std::unique_ptr<TIterator> catIter(cat("channelCat")->typeIterator());
  TObject* c;
  while( (c = catIter->Next()) ) {
    if(sample(sampleName,c->GetName())) sample(sampleName,c->GetName())->SetFillColor(in);
  }
}
void TRooWorkspace::sampleSetLineColor(const char* sampleName, Int_t in) {
  std::unique_ptr<TIterator> catIter(cat("channelCat")->typeIterator());
  TObject* c;
  while( (c = catIter->Next()) ) {
    if(sample(sampleName,c->GetName())) sample(sampleName,c->GetName())->SetLineColor(in);
  }
}

void TRooWorkspace::sampleScale(const char* sampleName,RooAbsReal& arg) {
  std::unique_ptr<TIterator> catIter(cat("channelCat")->typeIterator());
  TObject* c;
  while( (c = catIter->Next()) ) {
    if(sample(sampleName,c->GetName())) sample(sampleName,c->GetName())->Scale(arg);
  }
}

#include "TPRegexp.h"
#include "TRooFit/Utils.h"

RooSimultaneous* TRooWorkspace::model(const char* channels) {
  //builds the model for the given channels, putting them in a RooSimultaneous, then imports that and returns
  
  std::vector<TRegexp> patterns;
  TStringToken nameToken(channels,";");
  while(nameToken.NextToken()) {
      TString subName = (TString)nameToken;
      patterns.push_back(TRegexp(subName,true));
  }
  
  TRegexp pattern(channels,true);
  std::unique_ptr<TIterator> catIter(cat("channelCat")->typeIterator());
  TObject* c;
  
  TString simPdfName("simPdf");
  TString factoryString("");
  int nComps(0);
  
  while( (c = catIter->Next()) ) {
    bool pass=false;
    for(auto& pattern : patterns) if(TString(c->GetName()).Contains(pattern)) pass=true;
    if(pass==false) continue;
    if( !pdf(Form("%s_with_Constraints",c->GetName())) ) {
      import(*TRooFit::BuildModel( *channel(c->GetName()), *data("obsData") ));
    }
    nComps++;
    factoryString += Form(",%s=%s_with_Constraints",c->GetName(),c->GetName());
    simPdfName += Form("_%s",c->GetName());
    
    //remove from the stagedChannels list if its there (FIXME: is this a memory leak?)
    fStagedChannels.remove(*channel(c->GetName()),true/*silent*/,true/*match by name*/);
    
  }
  if(nComps>0) {
    //if(nComps==cat("channelCat")->numTypes()) { simPdfName="simPdf"; } //all channels available
    if(pdf(simPdfName)) return static_cast<RooSimultaneous*>(pdf(simPdfName));
    factory(Form("SIMUL::%s(channelCat%s)",simPdfName.Data(),factoryString.Data()));
    
   
    
    
    //if got here then need to create the global observables snapshot too ...
    //need list of gobs then ..
    RooAbsPdf* m = pdf(simPdfName);
     //infer the global observables, nuisance parameters, model args (const np) 
    RooArgSet* gobs_and_np = m->getParameters(*set("obs"));

    //remove the poi ...
    //if(set("poi")) gobs_and_np->remove(*set("poi"));

    RooArgSet gobs;
    gobs.add(*gobs_and_np); //will remove the np in a moment

    //now pass this to the getAllConstraints method ... it will modify it to contain only the np!
    RooArgSet* s = m->getAllConstraints(*set("obs"),*gobs_and_np);
     delete s; //don't ever need this 

     //gobs_and_np now only contains the np
     gobs.remove(*gobs_and_np);
     //ensure all global observables are held constant now - important for pll evaluation
     gobs.setAttribAll("Constant",kTRUE);
     
     saveSnapshot(Form("gobs%s",simPdfName(6,simPdfName.Length()).Data()),gobs);
     
     //save the list of parameters as a set (poi+np)
     RooArgSet np;RooArgSet args;
      std::unique_ptr<TIterator> itr(gobs_and_np->createIterator());
      RooAbsArg* arg = 0;
      while((arg=(RooAbsArg*)itr->Next())) { 
          if(arg->isConstant()) args.add(*arg);
          else np.add(*arg);
      }
      defineSet(Form("params%s",simPdfName(6,simPdfName.Length()).Data()),np);
      defineSet(Form("args%s",simPdfName(6,simPdfName.Length()).Data()),args);
     
     delete gobs_and_np;
     
  }
  
  return static_cast<RooSimultaneous*>(pdf(simPdfName));
}

//draw a channel's stack and overlay the data too
void TRooWorkspace::channelDraw(const char* channelName, const char* opt, const TRooFitResult& res) {

  channel(channelName)->Draw(opt,res);
  fDummyHists[channelName]->Reset();
  if(data("obsData")) {
    data("obsData")->fillHistogram(fDummyHists[channelName], *var(fDummyHists[channelName]->GetName()), Form("channelCat==channelCat::%s",channelName));
    //update all errors to usual poisson
    for(int i=1;i<=fDummyHists[channelName]->GetNbinsX();i++) fDummyHists[channelName]->SetBinError(i,sqrt(fDummyHists[channelName]->GetBinContent(i)));
    fDummyHists[channelName]->SetMarkerStyle(20);
    fDummyHists[channelName]->Draw("same");
  }

}

Bool_t TRooWorkspace::writeToFile(const char* fileName, Bool_t recreate) {
  //ensure all staged channels are imported ...
  std::unique_ptr<TIterator> itr( fStagedChannels.createIterator() );
  TObject* obj;
  while( (obj = itr->Next()) ) {
    import(*static_cast<RooAbsPdf*>(obj));
  }
  fStagedChannels.removeAll();
  
  return RooWorkspace::writeToFile(fileName,recreate);
  
}

#include "TCanvas.h"
#include "TROOT.h"

void TRooWorkspace::Draw(Option_t* option, const TRooFitResult& res) {
  //draws each channel
  TVirtualPad* pad = gPad;
  if(!pad) {
    gROOT->MakeDefCanvas();
    pad = gPad;
  }
  
  TString sOpt(option);
  sOpt.ToLower();
  if(!sOpt.Contains("same")) {
    pad->Clear();
  
    int nCat = cat("channelCat")->numTypes();
    if(nCat>1) {
      int nRows = nCat/sqrt(nCat);
      int nCols = nCat/nRows;
      if(nRows*nCols < nCat) nCols++;
      pad->Divide(nCols,nRows);
    }
  }
  
  
  TIterator* catIter = cat("channelCat")->typeIterator();
  TObject* c;
  int i=1;
  while( (c = catIter->Next()) ) {
    pad->cd(i++);
    channelDraw(c->GetName(),option,res);
  }
  pad->cd(0);

  delete catIter;

}



/*
std::pair<RooDataSet*,RooArgSet*> RooHypoModel::createToyDataSet(bool doBinned,const char* channels) {

   RooAbsPdf* thePdf = model(channels);

   //determine global observables for this model ...
   RooArgSet* gobs_and_np = model->getParameters(*set("obs"));

   //remove the poi ...
   gobs_and_np->remove(*set("poi"));

   RooArgSet gobs;
   gobs.add(*gobs_and_np); //will remove the np in a moment


   //now pass this to the getAllConstraints method ... it will modify it to contain only the np!
   RooArgSet* s = model->getAllConstraints(*set("obs"),*gobs_and_np);
   delete s; //don't ever need this 

   //gobs_and_np now only contains the np
   gobs.remove(*gobs_and_np);

   RooArgSet* toy_gobs = new RooArgSet;gobs.snapshot(*toy_gobs);
   //and then generate global and non-global observables 

   RooDataSet* globals = thePdf->generateSimGlobal(gobs,1);
   *toy_gobs = *globals->get(0);
   delete globals;

   RooDataSet* ddata;
   if(!thePdf->canBeExtended()) {
      //must be a counting experiment ... generate 1 dataset entry per category ... so must recurse through simultaneous pdf
      //usually only one simpdf though ... because user can use a RooSuperCategory to combine several categories!
      ddata = new RooDataSet("toyData","toyData",*set("obs"));
      buildDataset(thePdf,*ddata, false);
   } else {
      //just generate as normal 
      //have to first check if expected events are zero ... if it is, then the dataset is just empty ('generate' method fails when expectedEvents = 0)
      if(thePdf->expectedEvents(*set("obs"))==0) {
         ddata = new RooDataSet("toyData","toyData",*set("obs"));
      } else {
         //std::cout << " expected = " << thePdf->expectedEvents(*data()->dataset()->get()) << std::endl;
         ddata = (doBinned) ? thePdf->generate(*set("obs"),RooFit::Extended(),RooFit::AllBinned()) : thePdf->generate(*set("obs"),RooFit::Extended());
      }
   }

  std::pair<RooDataSet*,RooArgSet*> out = std::make_pair(ddata,toy_gobs);

   return out;

}


void RooHypoModel::buildDataset(RooAbsPdf* thePdf, RooDataSet& out, RooArgSet& gobs, bool doAsimov ) {
   //check if this pdf is a simultaneous 
   if(thePdf->IsA()==RooSimultaneous::Class()) {
      //need to get the observable set for each category 
      RooSimultaneous* simPdf = static_cast<RooSimultaneous*>(thePdf);
      //loop over index category 
      std::unique_ptr<TIterator> itr(simPdf->indexCat().typeIterator());
      RooCatType* tt = NULL;
      while((tt=(RooCatType*) itr->Next())) {
         std::cout << " in category " << tt->GetName() << " of " << thePdf->GetName() << std::endl;
         const_cast<RooArgSet*>(out.get())->setCatIndex(simPdf->indexCat().GetName(),tt->getVal());
         if(simPdf->getPdf(tt->GetName())) buildDataset(simPdf->getPdf(tt->GetName()), out, doAsimov);
      }
      return;
   }

   //got here, so not in a simultaneous 


   if(doAsimov) {
      //... so get the terms of the pdf that constrain the observables:
      RooArgSet params(poi()); params.add(np()); //the poi and np
      RooArgSet* constraints = thePdf->getAllConstraints(gobs,params);
      //constraints are now all the parts of the pdf that do not feature global observables ... so they must feature the observables!
      //loop over constraint pdfs, recognise gaussian, poisson, lognormal
      TIterator* citer = constraints->createIterator();
      RooAbsPdf* pdf = 0;
      while((pdf=(RooAbsPdf*)citer->Next())) { 
         //determine which obs this pdf constrains. There should only be one!
         std::unique_ptr<RooArgSet> cgobs(pdf->getObservables(*out.get()));
         if(cgobs->getSize()!=1) { std::cout << "constraint " << pdf->GetName() << " constrains " << cgobs->getSize() << " observables. skipping..." << std::endl; continue; }
         RooAbsArg* gobs_arg = cgobs->first();
   
         //now iterate over the servers ... the first server to depend on a nuisance parameter or the poi we assume is the correct server to evaluate ..
         std::unique_ptr<TIterator> itr(pdf->serverIterator());
         for(RooAbsArg* arg = static_cast<RooAbsArg*>(itr->Next()); arg != 0; arg = static_cast<RooAbsArg*>(itr->Next())) {
            RooAbsReal * rar = dynamic_cast<RooAbsReal *>(arg); 
            if( rar && ( rar->dependsOn(np()) || rar->dependsOn(poi()) ) ) {
//               std::cout << " setting " << gobs_arg->GetName() << " equal to value of " << arg->GetName() << " (" << rar->getVal() << ")" << std::endl;
               const_cast<RooArgSet*>(out.get())->setRealValue(gobs_arg->GetName(),rar->getVal());
            }
         }
      }
      delete citer;
      //now add the current values to the dataset 
      out.add(*out.get());
   } else {
      //just generate a single toy with the list of observables for all the things are actually depend on
      RooArgSet* obs = thePdf->getObservables(*out.get());
      RooDataSet* toy = thePdf->generate(*obs,1);
      const_cast<RooArgSet*>(out.get())->assignValueOnly(*toy->get(0));
      out.add(*out.get());
      delete toy;delete obs;
   }

}

*/