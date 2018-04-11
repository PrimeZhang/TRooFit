/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "TRooFit/TRooHF1.h" 


ClassImp(TRooHF1) 


TRooHF1::TRooHF1(const TRooHF1& other, const char* name) :  
   RooAbsReal(other,name), TRooAbsH1Fillable(other,this)
{
  //Copy constructor
}




Double_t TRooHF1::evaluate() const 
{
  double out1 = TRooAbsH1Fillable::evaluateImpl(false);
  return out1;

  //The main roofit evaluation method. Users should not call this directly 
  //It is computing the pdf value (the probability density) 

//std::cout << "rangeName = " << GetRangeName() << " "; fObservables.Print("v"); 

  double out = 0;
  int bin = getBin(GetName()); //forcefully use OUR binning 



    //loop over parameter snapshots, assessing which sets are valid (discrete params must match exactly)
    //if we find an exact match, we go with that
    
    //goal is that for each parameter to obtain the spacepoint IDs for the two closest valid spacepoints
    
    std::vector<int>& upSet = fStandardUpSet; //(fParameters.getSize(),-1); 
    std::vector<int>& downSet= fStandardDownSet; //(fParameters.getSize(),-1); 
    int nomSet = -1;
    
    int pset=-1;
    if(fParameters.getSize()==0) { 
      pset=0;
    } else {
    
      if(kIsStandardParameterVariations) {
      
        nomSet = 0;
      
      } else {
    
        int i=0;
        std::vector<int> setUp;std::vector<int> setDown; //get filled with parameter indices that should get set to up/down
        std::fill(fStandardUpSet.begin(),fStandardUpSet.end(),-1);std::fill(fStandardDownSet.begin(),fStandardDownSet.end(),-1);
        for(auto& parVals : fParameterSnapshots) {
          bool match(true); bool invalid(false);
          RooFIter parItr(fParameters.fwdIterator());
          int j=0; 
          setUp.clear();setDown.clear();
          while(auto par = parItr.next() ) {
            
            RooAbsCategory* cat = dynamic_cast<RooAbsCategory*>(par);
            if(cat) {
              if( cat->getIndex() != int(parVals[j]+0.5) ) {
                match=false;invalid=true;break;
              }
            } else {
              //record if this is up/down/nom variation for this parameter
              double parVal = ((RooAbsReal*)par)->getVal();
              double parDiff = fabs(parVal - parVals[j]);
              if(parDiff > 1e-9) {
                match=false;
                if(nomSet==-1) continue; //don't use the nomSet as our up-vs-down set 
                //check parVals[j] is different to fParameterSnapshots[nomSet][j] ... i.e. that this is a variation for parameter j
                if(fabs(parVals[j]-fParameterSnapshots[nomSet][j])<1e-9) continue;
                
                if(upSet[j]==-1) setUp.push_back(j); //the first variation we found
                else if(fInterpCode[j]==0) {
                  //special case for piecewise linear 
                  
                  if(downSet[j]==-1) {
                    double relativeValPos = (parVal - fParameterSnapshots[nomSet][j])/(fParameterSnapshots[upSet[j]][j]-fParameterSnapshots[nomSet][j]);
                    
                    double relativeCurrPos = (parVals[j] - fParameterSnapshots[nomSet][j])/(fParameterSnapshots[upSet[j]][j]-fParameterSnapshots[nomSet][j]);
                  
                    if((relativeValPos > 1 && relativeCurrPos > 0) || (relativeValPos>0 && relativeCurrPos>0 && relativeCurrPos<relativeValPos)) {
                      setDown.push_back(j); //add down 
                    } else if((relativeValPos < 0 && relativeCurrPos < 1) || (relativeValPos>0 && relativeCurrPos<1 && relativeCurrPos>relativeValPos)) {
                      setUp.push_back(j); //replace up
                    }
                  } else {
                    //upSet and downSet must be closer to actual val than nomVal ...
                    double relativeValPos = (parVal - fParameterSnapshots[downSet[j]][j])/(fParameterSnapshots[upSet[j]][j]-fParameterSnapshots[downSet[j]][j]);
                    double relativeCurrPos = (parVals[j] - fParameterSnapshots[downSet[j]][j])/(fParameterSnapshots[upSet[j]][j]-fParameterSnapshots[downSet[j]][j]);
                    
                    if( (relativeValPos < 0 && relativeCurrPos < 1) || (relativeValPos < 1 && relativeCurrPos < 1 && relativeValPos < relativeCurrPos) ) {
                      setUp.push_back(j); //replace up 
                    } else if(relativeValPos < 1 && relativeValPos > 0 && relativeCurrPos > 0 && relativeCurrPos < relativeValPos) {
                      setDown.push_back(j); //replace down
                    }
                    
                  }
                }
                else if(downSet[j]==-1) setDown.push_back(j);
                
                
              }
            }
            j++;
          }
          if(match) {
            pset=i; 
            break; //can just stop right now, found a perfect match
          }
          if(!invalid) { //if snapshot featured a variation up or down, store that
            if(nomSet==-1) nomSet=i; //the first valid set is used as the nom set
            //signal which parameters this snapshot is a valid variation for
            for(auto& j : setUp) upSet[j]=i;
            for(auto& j : setDown) downSet[j]=i;
          }
          i++;
        }
       }
    }
    
    //add the functional bin values
    if(fFunctionalBinValues.find(-1)!=fFunctionalBinValues.end()) {
      for(auto& vals : fFunctionalBinValues.at(-1)) {
        //warning: dont want to use getVal(_normSet) because we've added a pdf, not necessarily added a NORMALIZED pdf
        out += static_cast<RooAbsReal&>(fValues[vals]).getVal(); //if function is a pdf, we assume it is already a density!
      }
    }
    if(fFunctionalBinValues.find(bin) != fFunctionalBinValues.end()) {
      for(auto& vals : fFunctionalBinValues.at(bin)) {
        out += static_cast<RooAbsReal&>(fValues[vals]).getVal(); 
      }
    }
    

    if(pset!=-1) {
      //add the raw values too
      TH1* hist = GetHist(pset);
      
      //calculate bin volume, only if necessary though .. 
      double val(0);
      if(fObsInterpCode) {
        int bb[3]; hist->GetBinXYZ(bin,bb[0],bb[1],bb[2]);
        switch(hist->GetDimension()) {
          case 1: val = hist->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hist->GetXaxis()->GetBinCenter( bb[0] )); break;
          case 2: val = hist->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hist->GetXaxis()->GetBinCenter( bb[0] ), 
                                     (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hist->GetYaxis()->GetBinCenter( bb[1] ) ); break;
          case 3: val = hist->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hist->GetXaxis()->GetBinCenter( bb[0] ), 
                                     (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hist->GetYaxis()->GetBinCenter( bb[1] ),
                                     (fObsInterpCode&4)? static_cast<RooAbsReal&>(fObservables[2]).getVal() : hist->GetYaxis()->GetBinCenter( bb[2] ) ); break;
        }
      } else {
         val = hist->GetBinContent(bin);
      }
      out += val;
    } else if(nomSet!=-1) { //can only interpolate when there's a valid parameter spacepoint
      //got here, must interpolate
      //loop over parameters, and use upSet and downSet to compute interpolated result
      TH1* hist = GetHist(nomSet);
      double nomVal(0);
      if(fObsInterpCode) {
        int bb[3]; hist->GetBinXYZ(bin,bb[0],bb[1],bb[2]);
        switch(hist->GetDimension()) {
          case 1: nomVal = hist->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hist->GetXaxis()->GetBinCenter( bb[0] )); break;
          case 2: nomVal = hist->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hist->GetXaxis()->GetBinCenter( bb[0] ), 
                                     (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hist->GetYaxis()->GetBinCenter( bb[1] ) ); break;
          case 3: nomVal = hist->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hist->GetXaxis()->GetBinCenter( bb[0] ), 
                                     (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hist->GetYaxis()->GetBinCenter( bb[1] ),
                                     (fObsInterpCode&4)? static_cast<RooAbsReal&>(fObservables[2]).getVal() : hist->GetYaxis()->GetBinCenter( bb[2] ) ); break;
        }
      } else {
        nomVal = hist->GetBinContent(bin);
      }
      double val = nomVal;

      RooFIter parItr(fParameters.fwdIterator());
      int i=-1;
      while(auto par = parItr.next() ) {
        i++;
        int upIdx = (kIsStandardParameterVariations) ? fStandardUpSet[i] : upSet[i]; //the paramSet corresponding to one nearest fluctuation 
        
        if(!kIsStandardParameterVariations && par->InheritsFrom( RooAbsCategory::Class() ) ) continue;
        if(upIdx==-1) continue; //no variation for this parameter (need at least a point)
        
        double x_val = ((RooAbsReal*)par)->getVal();
        
        int downIdx = (kIsStandardParameterVariations) ? fStandardDownSet[i] : downSet[i]; //the paramSet that corresponds to the other nearest fluctuation (may be =-1 if no additional fluctuation)

        double x_up = fParameterSnapshots[upIdx][i];
        double x_down = (downIdx!=-1) ? fParameterSnapshots[downIdx][i] : fParameterSnapshots[nomSet][i];

        
        if(fInterpCode[i]==0 && downIdx!=-1) {
          //when using piecewise interpolation, we may prefer nominal to one of up or down, if nominal is "closer" 
          double x_nom = fParameterSnapshots[nomSet][i];
          //have 4 numbers: x_up, x_down, x_nom, x_val  
          //need to find the two which straddle x_val or are otherwise closest to x_val 
          if(x_down < x_nom && x_nom < x_up) {
            //standard ordering 
            if(x_nom < x_val) {
              //prefer nom to down
              x_down = x_nom; downIdx=-1;
            } else {
              //prefer nom to up
              upIdx = downIdx; x_up = x_down;
              downIdx=-1; x_down = x_nom;
            }
          } else if(x_up < x_nom && x_nom < x_down) {
            //reverse ordering 
            if(x_nom < x_val) {
              //prefer nom to up
              upIdx = downIdx; x_up = x_down;
              downIdx=-1; x_down = x_nom;
            } else {
              //prefer nom to down 
              x_down = x_nom; downIdx=-1;
            }
          } else {
            //unusual ordering ... not really supported ... 
            //we'd really have to check where x_val is relative to the three values 
            //also the x_up and x_down obtained may not straddle x_val, even if there were values that did
          }
          
        }
        
        //now calculate the value based on interpolation between these two points 
        double y_up(0);
        double y_down(nomVal);
        if(fObsInterpCode) {
          TH1* hh = fHists[upIdx];
          int bb[3]; hh->GetBinXYZ(bin,bb[0],bb[1],bb[2]);
          switch(hh->GetDimension()) {
            case 1: y_up = hh->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hh->GetXaxis()->GetBinCenter( bb[0] )); break;
            case 2: y_up = hh->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hh->GetXaxis()->GetBinCenter( bb[0] ), 
                                      (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hh->GetYaxis()->GetBinCenter( bb[1] ) ); break;
            case 3: y_up = hh->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hh->GetXaxis()->GetBinCenter( bb[0] ), 
                                      (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hh->GetYaxis()->GetBinCenter( bb[1] ),
                                      (fObsInterpCode&4)? static_cast<RooAbsReal&>(fObservables[2]).getVal() : hh->GetYaxis()->GetBinCenter( bb[2] ) ); break;
          }
          if(downSet[i]!=-1) {
            hh = fHists[downIdx];
            switch(hh->GetDimension()) {
              case 1: y_down = hh->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hh->GetXaxis()->GetBinCenter( bb[0] )); break;
              case 2: y_down = hh->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hh->GetXaxis()->GetBinCenter( bb[0] ), 
                                        (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hh->GetYaxis()->GetBinCenter( bb[1] ) ); break;
              case 3: y_down = hh->Interpolate((fObsInterpCode&1)? static_cast<RooAbsReal&>(fObservables[0]).getVal() : hh->GetXaxis()->GetBinCenter( bb[0] ), 
                                        (fObsInterpCode&2)? static_cast<RooAbsReal&>(fObservables[1]).getVal() : hh->GetYaxis()->GetBinCenter( bb[1] ),
                                        (fObsInterpCode&4)? static_cast<RooAbsReal&>(fObservables[2]).getVal() : hh->GetYaxis()->GetBinCenter( bb[2] ) ); break;
            }
          }
          
        } else {
          y_up = fHists[upIdx]->GetBinContent(bin);
          if(downIdx!=-1) y_down = fHists[downIdx]->GetBinContent(bin);
        }
        
        
        bool doCode2(false);
        switch(fInterpCode[i]*(downIdx!=-1)) {
        case 0:{ //piecewise linear always used if only one variation
             //if downSet unavailable, use nominal set as the down variation
            double tmpVal = ((y_up-y_down)/(x_up-x_down))*(x_val - x_down) + y_down;
            val += (tmpVal - nomVal);
            }break;
        case 2: //6th order poly with log extrapolation 
            doCode2=true;
        case 3:{ //6th order poly with linear extrapolation
            
            if(x_val < x_down && x_val < x_up) {
              if(doCode2) {
                val *= (x_down < x_up) ? std::pow(y_down/nomVal, x_val/x_down) : std::pow(y_up/nomVal, x_val/x_up);
              } else {
                //linear extrapolate from x_val and x_down or x_up, whichever is lower 
                if(x_down < x_up) {
                  y_up = nomVal; x_up = fParameterSnapshots[nomSet][i];
                } else {
                  y_down = nomVal; x_down = fParameterSnapshots[nomSet][i];
                }
                double tmpVal = ((y_up-y_down)/(x_up-x_down))*(x_val - x_down) + y_down;
                val += (tmpVal - nomVal);
              }
            } else if(x_val > x_up && x_val > x_down) {
              if(doCode2) {
                val *= (x_down < x_up) ? std::pow(y_up/nomVal, x_val/x_up) : std::pow(y_down/nomVal, x_val/x_down);
              } else {
                //linear extrapolate from x_val and x_down or x_up, whichever is lower 
                if(x_down < x_up) {
                  y_down = nomVal; x_down = fParameterSnapshots[nomSet][i];
                } else {
                  y_up = nomVal; x_up = fParameterSnapshots[nomSet][i];
                }
                double tmpVal = ((y_up-y_down)/(x_up-x_down))*(x_val - x_down) + y_down;
                val += (tmpVal - nomVal);
              }
            } else {
              //Code based off what is in HistFactor::PiecewiseInterpolation (interpCode 4)
            
              //scale and shift x values so x_up-x_down = 2 and x_up+x_down=0 (i.e. usual +1, -1 case) 
              //sf even flips x_up and x_down so that x_up > x_down
              //std::cout << "x_val =" << x_val << " x_up=" << x_up << " x_down=" << x_down << std::endl;
              double sf = 2.0/(x_up-x_down);
              x_val *= sf; x_up *= sf; x_down *= sf;
              double sh = (x_up+x_down)/2.0;
              x_val -= sh; 
              double eps_plus = y_up-nomVal; double eps_minus=nomVal-y_down;
              double S = 0.5 * (eps_plus + eps_minus);
              double A = 0.0625 * (eps_plus - eps_minus);
              val += x_val * (S + x_val * A * ( 15 + x_val * x_val * (-10 + x_val * x_val * 3  ) ) );
            }
                    
            }break;
        case 4:{ //6th order polynomial with log extrapolation
        
        
            //scale and shift x values so x_up-x_down = 2 and x_up+x_down=0 (i.e. usual +1, -1 case) 
            //sf even flips x_up and x_down so that x_up > x_down
            
            //std::cout << "x_val =" << x_val << " x_up=" << x_up << " x_down=" << x_down << std::endl;
            double sf = 2.0/(x_up-x_down);
            x_val *= sf; x_up *= sf; x_down *= sf;
            double sh = (x_up+x_down)/2.0;
            x_val -= sh; //x_up -= sh; x_down -= sh; //... x_up and x_down should equal 1 and -1 by this point
            
        
            if(x_val >= 1.) {
              val *= std::pow(y_up/nomVal, x_val);
            } else if(x_val <= -1.) {
              val *= std::pow(y_down/nomVal, -x_val);
            } else if(x_val) {
              //Based off what is in FlexibleInterpVar (interpCode 4)
              
              //coefficients can be precomputed, but are function of nomSet, upSet, downSet indices, bin, and i
              
              double coeff[6];
              
              double pow_up       =  y_up/nomVal;
              double pow_down     =  y_down/nomVal;
              double logHi        =  std::log(pow_up) ; //BUGFIXED!
              double logLo        =  std::log(pow_down); //BUGFIXED!
              double pow_up_log   = y_up <= 0.0 ? 0.0 : pow_up * logHi;
              double pow_down_log = y_down <= 0.0 ? 0.0 : -pow_down    * logLo;
              double pow_up_log2  = y_up <= 0.0 ? 0.0 : pow_up_log  * logHi;
              double pow_down_log2= y_down <= 0.0 ? 0.0 : -pow_down_log* logLo;
      
              double S0 = (pow_up+pow_down)/2;
              double A0 = (pow_up-pow_down)/2;
              double S1 = (pow_up_log+pow_down_log)/2;
              double A1 = (pow_up_log-pow_down_log)/2;
              double S2 = (pow_up_log2+pow_down_log2)/2;
              double A2 = (pow_up_log2-pow_down_log2)/2;
              
              //fcns+der+2nd_der are eq at bd
              
              // cache  coefficient of the polynomial 
              coeff[0] = 1./(8)        *(      15*A0 -  7*S1 + A2);
              coeff[1] = 1./(8)     *(-24 + 24*S0 -  9*A1 + S2);
              coeff[2] = 1./(4)*(    -  5*A0 +  5*S1 - A2);
              coeff[3] = 1./(4)*( 12 - 12*S0 +  7*A1 - S2);
              coeff[4] = 1./(8)*(    +  3*A0 -  3*S1 + A2);
              coeff[5] = 1./(8)*( -8 +  8*S0 -  5*A1 + S2);
              
              
              val *= (1. + x_val * (coeff[0] + x_val * (coeff[1] + x_val * (coeff[2] + x_val * (coeff[3] + x_val * (coeff[4] + x_val * coeff[5]) ) ) ) ) );
              
            }
            
            
            }break;
        }

        
      }

      out += val;
    }

  
  //multiply by all the norm factors
  RooFIter itr(fNormFactors.fwdIterator());
  while( RooAbsReal* arg = (RooAbsReal*)itr.next() ) out *= arg->getVal(); //NOTE: should we use _normSet? leads to issues if normfactor is a pdf...
  
  //and by the shape factors for this bin
  if(fBinsShapeFactors.find(bin)!=fBinsShapeFactors.end()) {
    for(auto& sfIdx : fBinsShapeFactors.at(bin)) {
      out *= ((RooAbsReal&)fShapeFactors[sfIdx]).getVal();
    }
  }
  
  if(kMustBePositive && out < fFloorValue) out=fFloorValue;
  
  return out;
  
}


