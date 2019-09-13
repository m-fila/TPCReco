#include "Track3D.h"

#include <iostream>
#include <algorithm>

#include <Fit/Fitter.h>
#include <Math/Functor.h>
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::addSegment(const TrackSegment3D & aSegment3D){

  mySegments.push_back(aSegment3D);
  update();
  
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::update(){

  myLenght = 0.0;  
  for(auto &aSegment: mySegments){
    myLenght += aSegment.getLength();
  }
  updateChi2();
  updateChargeProfile();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::vector<double> Track3D::getSegmentsStartEndXYZ() const{

  std::vector<double> coordinates;
  if(!mySegments.size()) return coordinates;
  
  for(auto &aSegment: mySegments){
    std::vector<double> segmentCoordinates = aSegment.getStartEndXYZ();
    coordinates.insert(coordinates.end(), segmentCoordinates.begin(), segmentCoordinates.begin()+3);
  }
  std::vector<double> segmentCoordinates = mySegments.back().getStartEndXYZ();
  coordinates.insert(coordinates.end(), segmentCoordinates.begin()+3, segmentCoordinates.end());
  
  return coordinates;  
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
double Track3D::getIntegratedCharge(double lambda) const{

  double charge = 0.0;
  double pastSegmentsLambda = 0.0;
  double segmentLambda = 0.0;
  for(const auto & aSegment: mySegments){
    segmentLambda = lambda - pastSegmentsLambda;
    if(segmentLambda>aSegment.getLength()) segmentLambda = aSegment.getLength();
    charge += aSegment.getIntegratedCharge(segmentLambda);
    pastSegmentsLambda += aSegment.getLength();
  }
  return charge;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::updateChargeProfile(){

  myChargeProfile.Set(0);
  if(getLength()<1.0) return;//FIXME threshold
    
  double h = 2; // [mm] FIXME optimize
  int nSteps =  getLength()/h;
  double lambdaCut = 0.0;
  double derivative = 0.0;

  myChargeProfile.SetPoint(0, 0, 0);
  for(int iStep=1;iStep<=nSteps;++iStep){
    lambdaCut = iStep*h;
    derivative = getIntegratedCharge(lambdaCut + h) - getIntegratedCharge(lambdaCut - h);
    derivative /= 2.0*h;
    myChargeProfile.SetPoint(myChargeProfile.GetN(),  lambdaCut, derivative);
  }
  myChargeProfile.SetPoint(myChargeProfile.GetN(), getLength(), 0);
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
double Track3D::getChi2() const{

  double chi2 = 0.0;
  std::for_each(segmentChi2.begin(), segmentChi2.end(), [&](auto aItem){chi2 += aItem;});
  return chi2;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::splitWorseChi2Segment(double lenghtFraction){

  update();
  if(!segmentChi2.size()) return;

  if(lenghtFraction<0 || lenghtFraction>1.0) lenghtFraction = 0.5;

  double maxChi2 = 0.0;
  unsigned int worseChi2Segment = 0;
  for(unsigned int iSegment=0;iSegment<mySegments.size();++iSegment){
    if(segmentChi2[iSegment]>maxChi2){
      maxChi2 = segmentChi2[iSegment];
      worseChi2Segment = iSegment;
    }
  }

  TrackSegment3D & aSegment = mySegments.at(worseChi2Segment);
  TVector3 aStep = aSegment.getLength()*aSegment.getTangent();
  aSegment.setStartEnd(aSegment.getStart(), aSegment.getStart() + lenghtFraction*aStep);

  TrackSegment3D aNewSegment = mySegments.at(worseChi2Segment);
  aNewSegment.setStartEnd(aSegment.getEnd(), aSegment.getEnd() + (1.0-lenghtFraction)*aStep);
  mySegments.insert(mySegments.begin()+worseChi2Segment+1, aNewSegment);
  
  update();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::updateChi2(){

 segmentChi2.clear();
  for(auto aItem: mySegments){
    segmentChi2.push_back(aItem.getRecHitChi2());
  }    
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::extendToWholeChamber(){

  if(!mySegments.size()) return;  
  double lambda = 100;

  TrackSegment3D & aFirstSegment = mySegments.front();
  TVector3 aStart = aFirstSegment.getStart() - lambda*aFirstSegment.getTangent();
  TVector3 aEnd = aFirstSegment.getEnd();
  aFirstSegment.setStartEnd(aStart, aEnd);
  
  TrackSegment3D & aLastSegment = mySegments.back();
  aStart = aLastSegment.getStart();
  aEnd = aLastSegment.getEnd() + lambda*aLastSegment.getTangent();  
  aLastSegment.setStartEnd(aStart, aEnd);
  
  update();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::shrinkToHits(){

  std::cout<<__FUNCTION__<<" begin"<<std::endl;

  double lambdaStart = 0.0;
  double lambdaEnd = getLength();
  if(getLength()<1.0) return;//FIXME move to configuration  
  double minChargeCut = 100.0;//FIXME move to configuration and (dynamically?) optimize
  double charge = 0.0;
  double h = 2.0;//FIXME move to configuration.
  while(charge<minChargeCut && lambdaStart<getLength()){
    lambdaStart +=h;
    charge = getChargeProfile().Eval(lambdaStart);  
  }
  charge = 0.0;
  while(charge<minChargeCut && lambdaEnd>0.0){
    lambdaEnd -=h;
    charge = getChargeProfile().Eval(lambdaEnd);  
  }
  if(lambdaEnd<0) lambdaEnd = 0.0;

  TrackSegment3D & aFirstSegment = mySegments.front();
  TrackSegment3D & aLastSegment = mySegments.back();
  
  TVector3 aStart = aFirstSegment.getStart() + getSegmentLambda(lambdaStart)*aFirstSegment.getTangent();  
  TVector3 aEnd = aLastSegment.getStart() + getSegmentLambda(lambdaEnd)*aLastSegment.getTangent();  
  
  aFirstSegment.setStartEnd(aStart, aFirstSegment.getEnd());
  aLastSegment.setStartEnd(aLastSegment.getStart(), aEnd);

  update();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
double Track3D::getSegmentLambda(double lambda) const{

  double segmentLambda = lambda;
  for(auto &aSegment: mySegments){
    if(segmentLambda<aSegment.getLength()) return segmentLambda;
    else segmentLambda -= aSegment.getLength();
  }
  return 0.0;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void Track3D::removeEmptySegments(){

  auto modifiedEnd = std::remove_if(mySegments.begin(), mySegments.end(), [](auto aItem){
									    std::vector<double> segmentParameters = aItem.getStartEndXYZ();
									    double segmentChi2 = aItem(segmentParameters.data());
									    return segmentChi2<1E-5;
									  });

  unsigned int newSize = modifiedEnd - mySegments.begin();
  mySegments.resize(newSize);

  update();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
double Track3D::chi2FromNodesList(const double *par){

  for(unsigned int iSegment=0;iSegment<mySegments.size();++iSegment){
    const double *segmentParameters = par+3*iSegment;
    mySegments.at(iSegment).setStartEnd(segmentParameters);
  }

  updateChi2();
  return getChi2();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
double Track3D::chi2FromSplitPoint(const double *par){

  double splitPoint = 0.5;
  if(par[0]>0.0 && par[0]<1.0) splitPoint = par[0];
  Track3D aTestTrack = *this;
  aTestTrack.splitWorseChi2Segment(splitPoint);  
  return aTestTrack.getChi2();
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
std::ostream & operator << (std::ostream &out, const Track3D &aTrack){

  out << "Number of segments: "<<aTrack.getSegments().size()<<std::endl;
  if(!aTrack.getSegments().size()) return out;

  std::cout<<"\t Path: start->end [chi2]: "<<std::endl;
  for(auto aSegment: aTrack.getSegments()) out<<"\t \t"<<aSegment<<std::endl;

  std::cout<<"\t Total track chi2: "<<aTrack.getChi2();  
  return out;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////