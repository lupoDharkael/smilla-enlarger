/* ----------------------------------------------------------------

SmillaEnlarger  -  resize, especially magnify bitmaps in high quality
    PointClass.h: simple vector classes

Copyright (C) 2009 Mischa Lusteck
Copyright (C) 2017 Alejandro Sirgo

This program is free software;
you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation;
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.

---------------------------------------------------------------------- */

#ifndef POINT_CLASS_H
#define POINT_CLASS_H

#include <math.h>
#include <iostream>
#include "ConstDefs.h"

using namespace std;

class PFloat {
public:
   float x;

   PFloat(void) {x=0.0;}
   PFloat(float f) {x=f;}
   ~PFloat(void) {}

   PFloat & operator=  (PFloat p) { x=p.x;         return *this;}
   PFloat & operator+= (PFloat p) { x+=p.x;        return *this;}
   PFloat & operator-= (PFloat p) { x-=p.x;        return *this;}
   PFloat & operator*= (float a)  { x*=a;          return *this;}
   PFloat   operator+  (PFloat p) { return  p+=*this;}
   PFloat   operator-  (PFloat p) { p.x=x-p.x;     return p;}
   PFloat   operator-  (void   )  { PFloat p(-x);  return p;}
   float    operator*  (PFloat p) { return x*p.x;  }
   PFloat   operator*  (float a)  { PFloat p(x*a); return p;}

   bool  IsZero(void) { return x==0.0; }
   PFloat Normalized(void) { return 1.0; }
   void  Normalize(void) { x = 1.0; }
   float Norm    ( void )  { return fabs(x); }
   float Norm1   ( void )  { return fabs(x); }
   void  Clip    ( void )  { if(x<0.0) x= 0.0; else if(x>1.0) x=1.0; }
   void  AddMul  ( float a, PFloat & p) {  x += a*p.x; }
   void  SetZero ( void  ) { x = 0.0;  }

   float toF( void ) { return x; };
 };

inline PFloat operator*(float a,PFloat p) { p*=a; return p; }


//-------------------------------------------------------------
//-------------------------------------------------------------

class Point {
public:
   float x,y,z;

   Point(void) {x=y=z=0.0;}
   Point(float f) {x=y=z=f;}
   Point(float x0,float y0,float z0) {x=x0;y=y0;z=z0;}
   ~Point(void) {}

   Point & operator=  (Point p) { x=p.x;y=p.y;z=p.z;return *this;}
   Point & operator+= (Point p) { x+=p.x;y+=p.y;z+=p.z;return *this;}
   Point & operator-= (Point p) { x-=p.x;y-=p.y;z-=p.z;return *this;}
   Point & operator*= (float a) { x*=a;y*=a;z*=a;return *this;}
   Point   operator+  (Point p) { return p+=*this;}
   Point   operator-  (Point p) { p.x=x-p.x;p.y=y-p.y;p.z=z-p.z;return p;}
   Point   operator-  (void   ) { Point p(-x,-y,-z);return p;}
   float   operator*  (Point p) { return x*p.x + y*p.y + z*p.z;}
   Point   operator*  (float a) { Point p(x*a,y*a,z*a);return p;}
   
   bool    IsZero(void) {return x==0.0 && y==0.0 && z==0.0;}
   Point   XProd(Point p) {
      Point q(y*p.z - z*p.y, z*p.x - x*p.z, x*p.y - y*p.x);return q;
   }
   Point   Normalized(void) {
      float d = sqrt(x*x+y*y+z*z);d = 1.0/d; return Point(x*d,y*d,z*d);
   }

   void   Normalize(void) {
      float d = sqrt(x*x+y*y+z*z);d = 1.0/d; x*=d;y*=d;z*=d;
   }

   float Norm(void) { return sqrt(x*x+y*y+z*z); }
   float Norm1(void) { return (1.0/3.0)*(fabs(x) + fabs(y) + fabs(z)); }
   void Clip(void) {
      if(x<0.0) x= 0.0; else if(x>1.0) x=1.0;
      if(y<0.0) y= 0.0; else if(y>1.0) y=1.0;
      if(z<0.0) z= 0.0; else if(z>1.0) z=1.0;
   }
   
   void FromColor(long col) {
      x = float( (col>>16) & 255 ) * ( 1.0/255.0 );
      y = float( (col>>8 ) & 255 ) * ( 1.0/255.0 );
      z = float( (col    ) & 255 ) * ( 1.0/255.0 );
   }
   
   void AddMul(float a, Point & p) {
      x += a*p.x;
      y += a*p.y;
      z += a*p.z;
   }
   
   void SetZero(void) {
      x = y = z = 0.0;
   }
 
   long ColorCast(void) { long ll,vv;
      vv  = long(x*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll  = vv<<16;
      vv  = long(y*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll += vv<<8;
      vv  = long(z*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll += vv;
      return ll;
   }
};

inline Point operator*(float a,Point p) {p*=a;return p;}


//-------------------------------------------------------------
//-------------------------------------------------------------

class Point4 {
public:
   float x,y,z,w;

   Point4(void) {x=y=z=w=0.0;}
   Point4(float f) {w=x=y=z=f;}
   Point4(float x0,float y0,float z0,float w0) {x=x0;y=y0;z=z0;w=w0;}
   ~Point4(void) {}

   Point4 & operator=  (Point4 p) { x=p.x;y=p.y;z=p.z;w=p.w; return *this;}
   Point4 & operator+= (Point4 p) { x+=p.x;y+=p.y;z+=p.z;w+=p.w; return *this;}
   Point4 & operator-= (Point4 p) { x-=p.x;y-=p.y;z-=p.z;w-=p.w; return *this;}
   Point4 & operator*= (float a)  { x*=a;y*=a;z*=a;w*=a; return *this;}
   Point4   operator+  (Point4 p) { return p+=*this;}
   Point4   operator-  (Point4 p) { p.x=x-p.x;p.y=y-p.y;p.z=z-p.z;p.w=w-p.w; return p;}
   Point4   operator-  (void   )  { Point4 p(-x,-y,-z,-w); return p;}
   float    operator*  (Point4 p) { return x*p.x + y*p.y + z*p.z + w*p.w;}
   Point4   operator*  (float a)  { Point4 p(x*a,y*a,z*a,w*a); return p;}

   bool    IsZero(void) {return x==0.0 && y==0.0 && z==0.0 && w==0.0;}
   Point4   Normalized(void) {
      float d = sqrt(x*x+y*y+z*z+w*w);d = 1.0/d; return Point4(x*d,y*d,z*d,w*d);
   }

   void   Normalize(void) {
      float d = sqrt(x*x+y*y+z*z+w*w);d = 1.0/d; x*=d;y*=d;z*=d;w*=d;
   }

   float Norm(void) { return sqrt(x*x+y*y+z*z+w*w); }
   float Norm1(void) { return (1.0/4.0)*(fabs(x) + fabs(y) + fabs(z) + fabs(w)); }
   void Clip(void) {
      if(x<0.0) x= 0.0; else if(x>1.0) x=1.0;
      if(y<0.0) y= 0.0; else if(y>1.0) y=1.0;
      if(z<0.0) z= 0.0; else if(z>1.0) z=1.0;
      if(w<0.0) w= 0.0; else if(w>1.0) w=1.0;
   }

   void FromColor(long col) {    // ARGB
      x = float( (col>>16) & 255 ) * ( 1.0/255.0 );
      y = float( (col>>8 ) & 255 ) * ( 1.0/255.0 );
      z = float( (col    ) & 255 ) * ( 1.0/255.0 );
      w = float( (col>>24) & 255 ) * ( 1.0/255.0 );
   }

   void AddMul(float a, Point4 & p) {
      x += a*p.x;
      y += a*p.y;
      z += a*p.z;
      w += a*p.w;
   }

   void SetZero(void) {
      x = y = z = w = 0.0;
   }

   long ColorCast(void) { long ll,vv;     // ARGB
      vv  = long(x*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll  = vv<<16;
      vv  = long(y*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll += vv<<8;
      vv  = long(z*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll += vv;
      vv  = long(w*255); vv = (vv<0 ? 0 : (vv>255 ? 255 : vv));
      ll += vv<<24;
      return ll;
   }
};

inline Point4 operator*(float a,Point4 p) {p*=a;return p;}


//-------------------------------------------------------------
//-------------------------------------------------------------


class Point2 {
public:
   float x,y;

   Point2(void) {x=y=0.0;}
   Point2(float f) {x=y=f;}
   Point2(float x0,float y0) {x=x0;y=y0;}
   ~Point2(void) {}

   Point2 & operator=  ( Point2 p ) { x=p.x;y=p.y;return *this;}
   Point2 & operator+= ( Point2 p ) { x+=p.x;y+=p.y;return *this;}
   Point2 & operator-= ( Point2 p ) { x-=p.x;y-=p.y;return *this;}
   Point2 & operator*= ( float  a ) { x*=a;y*=a;return *this;}
   Point2   operator+  ( Point2 p ) { return p+=*this;}
   Point2   operator-  ( Point2 p ) { p.x=x-p.x;p.y=y-p.y;return p;}
   Point2   operator-  ( void     ) { Point2 p(-x,-y);return p;}
   float    operator*  ( Point2 p ) { return x*p.x + y*p.y;}
   Point2   operator*  ( float  a ) { Point2 p(x*a,y*a);return p;}
   
   bool    IsZero(void) {return x==0.0 && y==0.0 ;}

   Point2   Normalized(void) {
      float d = sqrt(x*x+y*y);d = 1.0/d; return Point2(x*d,y*d);
   }

   void   Normalize(void) {
      float d = sqrt(x*x+y*y);
      if(d>0.0){
         d = 1.0/d; x*=d;y*=d;
      }
   }

   float Norm (void) { return (1.0/2.0) * ( fabs(x) + fabs(y) ); }
   float Norm1(void) { return sqrt(x*x+y*y); }
   void  Clip    ( void )  { }
   void AddMul(float a, Point2 & p) {  x += a*p.x;  y += a*p.y;  }
   void SetZero(void) {  x = y = 0.0; }

   Point2 DoubleDir(void) {    // angle of vector doubled, length unmodified
      Point2 q; 
      float d = sqrt(x*x+y*y); 
      if(d==0.0) return Point2(0.0,0.0);
      d = 1.0/d;
      q.x = ( x*x - y*y ) * d;
      q.y = 2*x*y  * d;
      return q;
   }

   Point2 DoubleDirNorm(void) {    // angle of vector doubled, length normalized
      Point2 q; 
      float d = (x*x+y*y); 
      if(d==0.0) return Point2(0.0,0.0);
      d = 1.0/d;
      q.x = ( x*x - y*y ) * d;
      q.y = 2*x*y  * d;
      return q;
   }
   
   float Angle(void) {
      float d,cc,ss,phi;
      d = sqrt(x*x+y*y);
      if(d==0.0) return 0.0;
      d = 1.0/d;
      cc = x*d;
      ss = y*d;
      if(cc > -0.8 && cc<0.8 ) {
         phi = acos(cc)*360.0*(0.5/PI);
         if(ss<0.0)
            phi = 360.0 - phi;
      }
      else {
         phi = asin(ss)*360.0*(0.5/PI);
         if(cc<0.0)
            phi = 180.0 - phi;
         if(phi<0.0)
            phi+=360.0;
      }
      return phi;   
   }
};

inline Point2 operator*(float a,Point2 p) {p*=a;return p;}

#endif
