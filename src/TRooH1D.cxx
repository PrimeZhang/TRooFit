
#include "TRooFit/TRooH1D.h"


//___________________________________
/* BEGIN_HTML
<p>A TRooH1D is the TRooFit PDF version of a TH1D</p>
<p>The value (returned by getVal) of a TRooH1D is a <b>density</b>, i.e. the object's value for a bin is the content / volume</p>
<p>In contrast, a TRooHF1D is not a density, and the value of that object is simply the content</p>

<p>You must first create a RooFit variable (e.g. RooRealVar or RooCategory) before you can
 construct a TRooH1D</p>
 
<p>Examples:
<pre>
RooRealVar x("x","x",0,10); //create a continuous variable with range 0->10
TRooH1D h("h","h",x,5); //create a 5-bin TRooH1D, that will be a pdf for "x"
<br>
RooCategory c("c","c");
c.defineType("A");c.defineType("B"); //create a discrete variable, values are A or B
TRooH1D h2("h2","h2",c); //creates a 2-bin TRooH1D, that will be a pdf for "c"
</pre>
</p>


<p>The histogram can then be filled like any normal histogram:
<pre>
h.Fill(4,3);
h.SetBinContent(4,3);
</pre>
</p>

<p>If the observable was a discrete variable (RooCategory) you can fill using the labels:
<pre>
h.Fill("A",3);
h.SetBinContent("A",3);
</pre>
</p>

<p>When you use the TRooAbsH1Fillable::Fill method, the histogram will automatically create a 
nuisance parameter to represent the statistical uncertainty for that bin. 
The nuisance parameter appears as a <i>shapeFactor</i> for that bin.</p>

<p>There are two types of factors, normFactors and shapeFactors:
<ul>
 <li><b><i>normFactors</i></b> : These multiply the content of every bin. Add one with TRooAbsH1::addNormFactor </li>
 <li><b><i>shapeFactors</i></b> : These multiply the content of a specific bin. Add one with TRooAbsH1::addShapeFactor</li>
 <li><b><i>transFactors (deprecated)</i></b> : These connect one TRooFit histogram to another, 
                  and are advanced use case (documentation to come).
                  There can only be one transFactor for a histogram.
                  If a histogram has a transFactor, the value of the histogram (before normFactor and shapeFactors)
                  is given by transFactor*denominatorHistogram</li>
</ul>

</p>
END_HTML */
//____________________________________


ClassImp(TRooH1D) 

void TRooH1D::Draw(Option_t* option) { 
    //Draw this TRooH1D
    //
    //See other Draw method for documentation and options
    
    TRooAbsH1::Draw(option); 
  }

void TRooH1D::Draw(Option_t* option,const TRooFitResult& r) 
{ 
    //Draw the TRooH1D, at the values of parameters given by the TRooFitResult's finalPars (r.floatParsFinal())
    //
    //Bin errors are calculated using the covariance matrix of the TRooFitResult
    //
    //Additional Options:
    //   e3XXX : Will draw an error band, with FillStyle=3XXX, and FillColor= histogram's LineColor
    //   init : Will use the TRooFitResult's initial parameter values (r.floatParsInit()) instead 
    //          .. final covariance matrix still used for errors
    //   pdf : Will draw TRooH1D as a TGraphErrors instead (samples 100 points), the value 
    //         of which will be the pdf density
    
    TRooAbsH1::Draw(option,r); 
  
}