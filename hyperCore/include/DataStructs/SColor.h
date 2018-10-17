#pragma once

/*****************************************************************************/ 
template < class S, int iMaxChannelValue >
class HYPERCORE_API TColor
{
public:
	TColor < S , iMaxChannelValue >()
	{
		r = g = b = 0.0;
		alpha = iMaxChannelValue ;
	}

	TColor < S , iMaxChannelValue >(S pr, S pg, S pb)
	{
		r = pr;
		g = pg;
		b = pb;
		alpha = iMaxChannelValue ;
	}

	TColor < S , iMaxChannelValue >(S pr, S pg, S pb, S palpha)
	{
		r = pr;
		g = pg;
		b = pb;
		alpha = palpha;
	}

	TColor < S , iMaxChannelValue >(const TColor < S , iMaxChannelValue >& rOther)
	{
		r = rOther.r;
		g = rOther.g;
		b = rOther.b;
		alpha = rOther.alpha;
	}

	TColor < S , iMaxChannelValue >(const char* pString, S palpha = 1.0)
	{
		fromHex(pString);
		alpha = palpha;
	}

	inline void set(S pr, S pg, S pb, S palpha)
	{
		r = pr;
		g = pg;
		b = pb;
		alpha = palpha;
	}
	
	inline void set(const TColor < S , iMaxChannelValue >& rOther)
	{
		r = rOther.r;
		g = rOther.g;
		b = rOther.b;
		alpha = rOther.alpha;
	}

	inline S operator[](int index) const
	{
		if(index == 0) 
			return r;
		else if(index == 1) 
			return g;
		else if(index == 2) 
			return b;
		else
			return alpha;
	}

	inline void setComponent(int index, S sValue)
	{
		if(index == 0) 
			r = sValue;
		else if(index == 1) 
			g = sValue;
		else if(index == 2) 
			b = sValue;
		else
			alpha = sValue;
	}


	inline void abs()
	{
		if(r < 0)
			r *= -1.0;
		if(g < 0)
			g *= -1.0;
		if(b < 0)
			b *= -1.0;
		if(alpha < 0)
			alpha *= -1.0;
	}

	inline void floor()
	{
		r = (int)r;
		g = (int)g;
		b = (int)b;
		alpha = (int)alpha;
	}

	inline void wrapToRange()
	{
		if(r < 0 || r > iMaxChannelValue)
			r = HyperCore::mantissa(fabs(r));
		if(g < 0 || g > iMaxChannelValue)
			g = HyperCore::mantissa(fabs(g));
		if(b < 0 || b > iMaxChannelValue)
			b = HyperCore::mantissa(fabs(b));
		alpha = HyperCore::clampToRange<S>(0, iMaxChannelValue, alpha);
	}

	inline void clamp()
	{
		r = HyperCore::clampToRange<S>(0, iMaxChannelValue, r);
		g = HyperCore::clampToRange<S>(0, iMaxChannelValue, g);
		b = HyperCore::clampToRange<S>(0, iMaxChannelValue, b);
		alpha = HyperCore::clampToRange<S>(0, iMaxChannelValue, alpha);
	}

	// Operators
	inline bool operator==(const TColor < S , iMaxChannelValue >& src) const 
	{ 
		return (bool)(src.r==r && src.g==g && src.b==b && src.alpha == alpha); 
	}

	inline bool operator!=(const TColor < S , iMaxChannelValue >& src) const 
	{ 
		return (bool)(src.r!=r || src.g!=g || src.b!=b || src.alpha != alpha); 
	}
	
	inline void operator +=(const TColor < S , iMaxChannelValue >& src)
	{
		r+=src.r;
		g+=src.g;
		b+=src.b;
	}
	
	inline void operator *=(const TColor < S , iMaxChannelValue >& src)
	{
		r*=src.r/(FLOAT_TYPE)(iMaxChannelValue);
		g*=src.g/(FLOAT_TYPE)(iMaxChannelValue);
		b*=src.b/(FLOAT_TYPE)(iMaxChannelValue);
	}

	inline void operator -=(const TColor < S , iMaxChannelValue >& src)
	{
		r-=src.r;
		g-=src.g;
		b-=src.b;
	}

	inline void operator *=(S scalar)
	{
		r*=scalar;
		g*=scalar;
		b*=scalar;
	}

	inline void operator /=(S scalar)
	{
		r/=scalar*(FLOAT_TYPE)(iMaxChannelValue);
		g/=scalar*(FLOAT_TYPE)(iMaxChannelValue);
		b/=scalar*(FLOAT_TYPE)(iMaxChannelValue);
	}

	inline TColor < S , iMaxChannelValue > operator *(const TColor < S , iMaxChannelValue >& src) const
	{
		return TColor < S , iMaxChannelValue >(r*src.r/(FLOAT_TYPE)(iMaxChannelValue), 
			g*src.g/(FLOAT_TYPE)(iMaxChannelValue), b*src.b/(FLOAT_TYPE)(iMaxChannelValue));	
	}

	inline TColor < S , iMaxChannelValue > operator /(const TColor < S , iMaxChannelValue >& src) const
	{
		return TColor < S , iMaxChannelValue >(r/src.r*(FLOAT_TYPE)(iMaxChannelValue), 
			g/src.g*(FLOAT_TYPE)(iMaxChannelValue), b/src.b*(FLOAT_TYPE)(iMaxChannelValue));	
	}

	inline TColor < S , iMaxChannelValue > operator *(S scalar) const 
	{	
		return TColor < S , iMaxChannelValue >(r*scalar, g*scalar, b*scalar); 
	}

	inline TColor < S , iMaxChannelValue > operator /(S scalar) const 
	{	
		return TColor < S , iMaxChannelValue >(r/scalar, g/scalar, b/scalar); 
	}

	inline TColor < S , iMaxChannelValue > operator +(const TColor < S , iMaxChannelValue >& src) const 
	{ 
		return TColor < S , iMaxChannelValue >(r + src.r, g + src.g, b + src.b); 
	}

	inline TColor < S , iMaxChannelValue > operator -(const TColor < S , iMaxChannelValue >& src) const 
	{ 
		return TColor < S , iMaxChannelValue >(r - src.r, g - src.g, b - src.b); 
	}

	inline TColor < S , iMaxChannelValue > operator +(S scalar) const 
	{ 
		return TColor < S , iMaxChannelValue >(r + scalar, g + scalar, b + scalar); 
	}

	inline TColor < S , iMaxChannelValue > operator -(S scalar) const 
	{	
		return TColor < S , iMaxChannelValue >(r - scalar, g - scalar, b - scalar); 
	}

	inline void operator=(const TColor < S , iMaxChannelValue >& src)
	{
		r = src.r;
		g = src.g;
		b = src.b;
		alpha = src.alpha;
	}

	inline void premultiplyAlpha()
	{
		r = r*alpha/(FLOAT_TYPE)iMaxChannelValue;
		g = g*alpha/(FLOAT_TYPE)iMaxChannelValue;
		b = b*alpha/(FLOAT_TYPE)iMaxChannelValue;
	}

	inline TColor < S , iMaxChannelValue > operator /(const TColor < S , iMaxChannelValue > *src) const
	{
		return TColor < S , iMaxChannelValue >(r/src->r*(FLOAT_TYPE)(iMaxChannelValue), 
			g/src->g*(FLOAT_TYPE)(iMaxChannelValue), b/src->b*(FLOAT_TYPE)(iMaxChannelValue));	
	}

/*
	inline void operator +=(const TColor < S , iMaxChannelValue > *src)
	{
		r+=src->r;
		g+=src->g;
		b+=src->b;
	}

	inline void operator -=(const TColor < S , iMaxChannelValue > *src)
	{
		r-=src->r;
		g-=src->g;
		b-=src->b;
	}
*/

	inline void copyFrom(const TColor < S , iMaxChannelValue >& src)
	{
		r = src.r;
		g = src.g;
		b = src.b;
		alpha = src.alpha;
	}

	inline S maxComponent() const 
	{ 
		return max(r, max(g,b)); 
	}

	inline S minComponent() const 
	{	
		return min(r, min(g,b)); 
	}

	inline S avg(void) const 
	{ 
		return (r + g + b)/3.0; 
	}

	inline S avg_grad_weighted(void) const
	{
		// weighed average for the Colors
		return (0.27*r + 0.67*g + 0.06*b);
	}

	inline void interpolateFrom(const TColor < S , iMaxChannelValue >& scolColor1, const TColor < S , iMaxChannelValue >& scolColor2, FLOAT_TYPE fInterpFromColor1)
	{
		r = HyperCore::interpolateFloat(scolColor1.r, scolColor2.r, fInterpFromColor1);
		g = HyperCore::interpolateFloat(scolColor1.g, scolColor2.g, fInterpFromColor1);
		b = HyperCore::interpolateFloat(scolColor1.b, scolColor2.b, fInterpFromColor1);
		alpha = HyperCore::interpolateFloat(scolColor1.alpha, scolColor2.alpha, fInterpFromColor1);
	}

	FLOAT_TYPE normalize(void)
	{
		FLOAT_TYPE length = r*r+g*g+b*b;

		length = F_SQRT(length);

		if(length==0.0)
			return 0.0;

		r /= length;
		g /= length;
		b /= length;

		return length;
	}

	void fromHSB(FLOAT_TYPE h, FLOAT_TYPE s, FLOAT_TYPE v);
	void toHSB(FLOAT_TYPE& dH, FLOAT_TYPE& dS, FLOAT_TYPE& dB) const;	
	// Hue-Chroma-Luma
	void toHCL(FLOAT_TYPE& fH, FLOAT_TYPE& fC, FLOAT_TYPE& fL) const;
	void fromHCL(FLOAT_TYPE fH, FLOAT_TYPE fC, FLOAT_TYPE fL);
	void toCMYK(FLOAT_TYPE& fC, FLOAT_TYPE& fM, FLOAT_TYPE& fY, FLOAT_TYPE& fK) const;
	void fromCMYK(FLOAT_TYPE fC, FLOAT_TYPE fM, FLOAT_TYPE fY, FLOAT_TYPE fK);
	void toHLS(FLOAT_TYPE& Hue, FLOAT_TYPE& Lumination, FLOAT_TYPE& Saturation);
	void fromHLS(FLOAT_TYPE Hue, FLOAT_TYPE Lumination, FLOAT_TYPE Saturation);
	// format: rgb[a]. note no #. Optional alpha.
	bool fromHex(const char* pString);
	void toHex(string& strOut, bool bIncludeAlpha) const;
	void toHex(char* pcsBuffOut, bool bIncludeAlpha) const;

	bool fromNamedColor(const char* pString);
	bool fromCSS(const char* pString);

	inline void toXYZ(SVector3D& svOut) const
	{
		FLOAT_TYPE var_R = r;
		FLOAT_TYPE var_G = g;
		FLOAT_TYPE var_B = b;

		if ( var_R > 0.04045 ) 
			var_R = pow((FLOAT_TYPE)( ( var_R + 0.055 ) / 1.055 ), (FLOAT_TYPE)2.4);
		else                   
			var_R = var_R / 12.92;
		if ( var_G > 0.04045 ) 
			var_G = pow((FLOAT_TYPE)( ( var_G + 0.055 ) / 1.055 ), (FLOAT_TYPE)2.4);
		else                   
			var_G = var_G / 12.92;
		if ( var_B > 0.04045 ) 
			var_B = pow((FLOAT_TYPE)( ( var_B + 0.055 ) / 1.055 ), (FLOAT_TYPE)2.4);
		else                   
			var_B = var_B / 12.92;

		var_R = var_R * 100.0;
		var_G = var_G * 100.0;
		var_B = var_B * 100.0;

		//Observer. = 2°, Illuminant = D65
		// Assumes sRGB source color space.
		svOut.x = var_R * 0.4124564 + var_G * 0.3575761  + var_B * 0.1804375;
		svOut.y = var_R * 0.2126729 + var_G * 0.7151522 + var_B * 0.0721750;
		svOut.z = var_R * 0.0193339 + var_G * 0.1191920 + var_B * 0.9503041;
	}

	inline void fromXYZ(const SVector3D& svIn)
	{
		FLOAT_TYPE var_X = svIn.x / 100.0;        //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
		FLOAT_TYPE var_Y = svIn.y / 100.0; //Y from 0 to 100.000
		FLOAT_TYPE var_Z = svIn.z / 100.0;        //Z from 0 to 108.883

		FLOAT_TYPE var_R = var_X *  3.2404542 + var_Y * -1.5371385 + var_Z * -0.4985314;
		FLOAT_TYPE var_G = var_X * -0.9692660 + var_Y *  1.8760108 + var_Z *  0.0415560;
		FLOAT_TYPE var_B = var_X *  0.0556434 + var_Y * -0.2040259 + var_Z *  1.0572252;

		if ( var_R > 0.0031308 ) 
			var_R = 1.055 * ( pow(var_R , (FLOAT_TYPE)( 1.0 / 2.4 ) ) ) - 0.055;
		else                     
			var_R = 12.92 * var_R;
		if ( var_G > 0.0031308 ) 
			var_G = 1.055 * ( pow( var_G , (FLOAT_TYPE)( 1.0 / 2.4 ) ) ) - 0.055;
		else                     
			var_G = 12.92 * var_G;
		if ( var_B > 0.0031308 ) 
			var_B = 1.055 * ( pow( var_B , (FLOAT_TYPE)( 1.0 / 2.4 ) ) ) - 0.055;
		else                     
			var_B = 12.92 * var_B;

		r = var_R;
		g = var_G;
		b = var_B;
	}

	static unsigned char convertFloatToChar(FLOAT_TYPE fValue)
	{
		return (char)(fValue*255.0);
	}

	static void XYZtoXY(const SVector3D& svXyz, FLOAT_TYPE &fXOut, FLOAT_TYPE& fYOut)
	{
		FLOAT_TYPE fSum = svXyz.x + svXyz.y + svXyz.z;
		if(fabs(fSum) > FLOAT_EPSILON)
		{
			fXOut = svXyz.x/fSum;
			fYOut = svXyz.y/fSum;
		}
		else
		{
			fXOut = fYOut = 0;
		}
	}

	static void xyYtoXYZ(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fYY, SVector3D& svXyzOut)
	{
		if(fabs(fY) > FLOAT_EPSILON)
		{
			svXyzOut.x = fYY/fY*fX;
			svXyzOut.y = fYY;
			svXyzOut.z = fYY/fY*(1.0 - fX - fY);
		}
		else
			svXyzOut.set(0, fYY, 0);
	}

	static void XYZtoLMS(const SVector3D& svXyz, SVector3D& svLms)
	{
		SMatrix2D smMatrix;
		smMatrix.makeXyzToLmsMatrix();
		svLms = smMatrix*svXyz;
	}

	static void LMStoXYZ(const SVector3D& svLms, SVector3D& svXyz)
	{
		SMatrix2D smMatrix;
		smMatrix.makeXyzToLmsMatrix();
		smMatrix.invert();
		svXyz = smMatrix*svLms;
	}

	inline FLOAT_TYPE lengthSquaredNoAlpha() const 
	{ 
		return r*r + g*g + b*b; 
	}
	
	inline void blendBetween(TColor < S , iMaxChannelValue >& from, TColor < S , iMaxChannelValue >& to, FLOAT_TYPE fInterp)
	{		
		TColor < S , iMaxChannelValue > t1, t2;
		t1 = from*(1.0 - fInterp);
		t2 = to*fInterp;
		*this = t1 + t2;
	}

	inline void toFloatString(string& strOut) const 
	{
		char pcsBuff[128];
		sprintf(pcsBuff, "{ %g, %g, %g, %g }", r, g, b, alpha);
		strOut = pcsBuff;
	}

	void fromTemperature(FLOAT_TYPE fTemperature);
	void adjustColorBalance(FLOAT_TYPE fCyanRedShadows, FLOAT_TYPE fMagentaGreeShadows, FLOAT_TYPE fYellowBlueShadows, 
							FLOAT_TYPE fCyanRedMidtones, FLOAT_TYPE fMagentaGreeMidtones, FLOAT_TYPE fYellowBlueMidtones, 
							FLOAT_TYPE fCyanRedHighlights, FLOAT_TYPE fMagentaGreeHighlights, FLOAT_TYPE fYellowBlueHighlights, 
							bool bPreserveLuminocity);

	// Expected format: { val, val, val, val }
	bool fromFloatString(const string& strText);

	static void ensureColorMapInitialized();

	static const TColor White;
	static const TColor Black;
	static const TColor Red;
	static const TColor Green;
	static const TColor Blue;
	static const TColor TransparentBlack;
	static const TColor TransparentWhite;
	static const TColor Magenta;
	static const TColor BabyBlue;

	static map < string, TColor<S, iMaxChannelValue> > *theColorNameMap;

	// data components
	S r,g,b,alpha;
};
/*****************************************************************************/
template <class S, int iMaxChannelValue > inline TColor<S,iMaxChannelValue > operator +(S scalar, TColor<S,iMaxChannelValue >& src)
{
	return TColor<S,iMaxChannelValue >(scalar + src.r,scalar + src.g,scalar + src.b);
}
template <class S, int iMaxChannelValue > inline TColor<S,iMaxChannelValue > operator -(S scalar, TColor<S,iMaxChannelValue >& src)
{
	return TColor<S,iMaxChannelValue >(scalar - src.r,scalar - src.g,scalar - src.b);
}
template <class S, int iMaxChannelValue > inline TColor<S,iMaxChannelValue > operator *(S scalar, TColor<S,iMaxChannelValue >& src)
{
	return TColor<S,iMaxChannelValue >(scalar*src.r,scalar*src.g,scalar*src.b);
}
/*****************************************************************************/ 
typedef TColor < float, 1 > SColor;
/*****************************************************************************/ 
typedef vector < SColor > TColorVector;
/*****************************************************************************/