#pragma once

#include "Math/MaVec2d.h"

//////////////////////////////////////////////////////////////////////////
// GaPositionUtility
class GaPositionUtility
{
public:
	enum Alignment
	{
		LEFT = 0x01,
		RIGHT = 0x02,
		HCENTRE = 0x04,
		TOP = 0x10,
		BOTTOM = 0x20,
		VCENTRE = 0x40,

		DEFAULT = LEFT | TOP,
		HFLAGS = LEFT | RIGHT | HCENTRE,
		VFLAGS = TOP | BOTTOM | VCENTRE,
	};

	/**
	 * Get a position to render at given @a Position, @a Size, and @a Alignment from
	 * within given @a Dimensions.
	 * @param Dimensions Dimensions to work inside of.
	 * @param Position Where from specified alignment to be positioned.
	 * @param Size Size of area to align. If either element is < 0.0f, then @a Dimension value will be assumed.
	 * @param Alignment Appropriate alignment to use.
	 * @return Top left point that meets requirements.
	 */
	static MaVec2d GetPosition( MaVec2d Dimensions, MaVec2d Position, MaVec2d Size, BcU32 AlignmentFlags );

	/**
	 * As for GetPosition, but passes through screen dimensions automatically.
	 */
	static MaVec2d GetScreenPosition( MaVec2d Position, MaVec2d Size, BcU32 AlignmentFlags );
};
