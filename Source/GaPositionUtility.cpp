#include "GaPositionUtility.h""

#include "System/Os/OsCore.h"
#include "System/Os/OsClient.h"

#include "Base/BcMath.h"

//static 
MaVec2d GaPositionUtility::GetPosition( MaVec2d Dimensions, MaVec2d Position, MaVec2d Size, BcU32 AlignmentFlags )
{
	BcAssert( BcBitsSet( AlignmentFlags & HFLAGS ) == 1 );
	BcAssert( BcBitsSet( AlignmentFlags & VFLAGS ) == 1 );

	const BcU32 HFlags = AlignmentFlags & HFLAGS;
	const BcU32 VFlags = AlignmentFlags & VFLAGS;

	if( Size.x() < 0.0f )
	{
		Size.x( Dimensions.x() );
	}

	if( Size.y() < 0.0f )
	{
		Size.y( Dimensions.y() );
	}

	MaVec2d RetVal;
	switch( HFlags )
	{
	case LEFT:
		RetVal.x( Position.x() );
		break;
	case RIGHT:
		RetVal.x( Dimensions.x() - ( Position.x() + Size.x() ) );
		break;
	case HCENTRE:
		RetVal.x( ( Dimensions.x() - Size.x() ) * 0.5f );
		break;
	default:
		BcBreakpoint;
	}

	switch( VFlags )
	{
	case TOP:
		RetVal.y( Position.y() );
		break;
	case BOTTOM:
		RetVal.y( Dimensions.y() - ( Position.y() + Size.y() ) );
		break;
	case VCENTRE:
		RetVal.y( ( Dimensions.y() - Size.y() ) * 0.5f );
		break;
	default:
		BcBreakpoint;
	}

	return RetVal;
}

//static
MaVec2d GaPositionUtility::GetScreenPosition( MaVec2d Position, MaVec2d Size, BcU32 AlignmentFlags )
{
	OsClient* Client = OsCore::pImpl()->getClient( 0 );
	MaVec2d Dimensions( Client->getWidth(), Client->getHeight() );

	return GetPosition( Dimensions, Position, Size, AlignmentFlags );
}
