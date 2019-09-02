#ifndef _TrackSegment3D_H_
#define _TrackSegment3D_H_

#include <string>
#include <vector>
#include <memory>

#include "TVector3.h"
#include "TH2D.h"

#include "Hit2D.h"
#include "TrackSegment2D.h"
#include "CommonDefinitions.h"

class TrackSegment3D{

public:

  TrackSegment3D(){};

  ~TrackSegment3D(){};

  void setBiasTangent(const TVector3 & aBias, const TVector3 & aTangent);

  void setStartEnd(const TVector3 & aStart, const TVector3 & aEnd);

  void setRecHits(const std::vector<TH2D> & aRecHits);

  ///Unit tangential vector along segment.
  const TVector3 & getTangent() const { return myTangent;}

  ///Bias vector perpendicular to tangent.
  const TVector3 & getBias() const { return myBias;}

  ///Bias vector with X=0.
  const TVector3 & getBiasAtX0() const { return myBiasAtX0;}

  ///Bias vector with Y=0.
  const TVector3 & getBiasAtY0() const { return myBiasAtY0;}

  ///Bias vector with Z=0.
  const TVector3 & getBiasAtZ0() const { return myBiasAtZ0;}

  ///Bias vector at the beggining of the segment.
  const TVector3 & getStart() const { return myStart;}

  ///Bias vector at the end of the segment.
  const TVector3 & getEnd() const { return myEnd;}

  ///Return given 2D projection 
  TrackSegment2D get2DProjection(int iDir) const;

  double getLength() const { return myLenght;}

  ///Return sum of chi2 for all projections.
  double getRecHitChi2() const;

  ///Operator needed for fitting.
  double operator() (const double *par);

private:

  TVector3 getPointOn2DProjection(double lambda, int strip_dir) const;

  ///Calculate vector for different parametrisations.
  void initialize();
 
  TVector3 myTangent, myBias;
  TVector3 myBiasAtX0, myBiasAtY0, myBiasAtZ0;
  TVector3 myStart, myEnd;
  double myLenght;

  std::vector<Hit2DCollection> myRecHits;

  
  
};


typedef std::vector<TrackSegment3D> TrackSegment3DCollection;

#endif
