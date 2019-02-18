/*=============================================================================
	FrColRibb.cpp: WColorRibbon implementation.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
	WColorRibbon implementation.
-----------------------------------------------------------------------------*/

//
// Ribbon constructor.
//
WColorRibbon::WColorRibbon( WContainer* InOwner, WWindow* InRoot )
	:	WWidget( InOwner, InRoot ),
		Curve( nullptr ),
		Ribbon( nullptr ),
		iSelected( -1 )
{
	// Widget implementation.
	Size.Width	= 300;
	Size.Height	= 32;

	// Create bitmap.
	Ribbon					= new TStaticBitmap();
	Ribbon->Format			= BF_RGBA;
	Ribbon->USize			= 256;
	Ribbon->VSize			= 1;
	Ribbon->UBits			= IntLog2(Ribbon->USize);
	Ribbon->VBits			= IntLog2(Ribbon->VSize);
	Ribbon->Filter			= BFILTER_Nearest;
	Ribbon->BlendMode		= BLEND_Regular;
	Ribbon->RenderInfo		= -1;
	Ribbon->PanUSpeed		= 0.f;
	Ribbon->PanVSpeed		= 0.f;
	Ribbon->Saturation		= 1.f;
	Ribbon->AnimSpeed		= 0.f;
	Ribbon->bDynamic		= true;
	Ribbon->bRedrawn		= false;
	Ribbon->Data.setSize(256*sizeof(TColor));
}


//
// Ribbon destructor.
//
WColorRibbon::~WColorRibbon()
{
	freeandnil(Ribbon);
}


//
// Set editor curve.
//
void WColorRibbon::SetCurve( TInterpCurve<TColor>* InCurve )
{
	Curve = InCurve;
	iSelected = -1;
	UpdateRibbon();
}


//
// User release mouse button.
//
void WColorRibbon::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnMouseUp( Button, X, Y );
}


//
// Mouse leave client area.
//
void WColorRibbon::OnMouseLeave()
{
	WWidget::OnMouseLeave();
}


//
// Mouse moves on ribbon.
//
void WColorRibbon::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnMouseMove( Button, X, Y );

	if( Button == MB_Left && iSelected != -1 )
	{
		// Move sample.
		TColor OldOutput = Curve->Samples[iSelected].Output;
		Curve->Samples.removeShift(iSelected);
		iSelected = Curve->AddSample( Float(Clamp(X-5, 0, Size.Width-11))/Float(Size.Width), OldOutput );

		OnChange();
		UpdateRibbon();
	}

	// Just highlight.
	if( Button == MB_None )
		iSelected = GetMarkerAt(X, Y);
}



//
// User click on ribbon.
//
void WColorRibbon::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnMouseDown( Button, X, Y );

	if( !Curve || Y < Size.Height-17 )
		return;

	if( Button == MB_Left )
	{
		// Add or highlight the sample.
		Int32 iCandidate = GetMarkerAt( X, Y );
		if( iCandidate == -1 )
		{
			// Add a new sample.
			Float Input = Float(Clamp(X-5, 0, Size.Width-11))/Float(Size.Width);
			iSelected = Curve->AddSample( Input, Curve->SampleLinearAt(Input, COLOR_Black) );
			OnChange();
			UpdateRibbon();
		}
		else
		{
			// Select sample.
			iSelected = iCandidate;
		}
	}
	else if( Button == MB_Right )
	{
		// Remove selected sample.
		Int32 iSample = GetMarkerAt( X, Y );
		if( iSample != -1 && Curve->Samples.size() > 1 )
		{
			Curve->Samples.removeShift(iSample);
			iSelected = -1;
			UpdateRibbon();
			OnChange();
		}
	}
}


//
// User double click on ribbon.
//
void WColorRibbon::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnDblClick( Button, X, Y );

	if( Button == MB_Left && iSelected != -1 )
	{
		WColorChooser::SharedColor = Curve->Samples[iSelected].Output;
		new WColorChooser( Root, false, TNotifyEvent( this, (TNotifyEvent::TEvent)&WColorRibbon::ColorSelected ) );
	}
}


//
// When user pick sample color.
//
void WColorRibbon::ColorSelected( WWidget* Sender )
{
	assert(iSelected != -1);
	Curve->Samples[iSelected].Output = WColorChooser::SharedColor;
	UpdateRibbon();
	OnChange();
}


//
// Return marker index in specified location or -1 if no marker found.
//
Int32 WColorRibbon::GetMarkerAt( Int32 X, Int32 Y )
{
	if( Curve )
	{
		if( Y < Size.Height-17 )
			return -1;

		for( Int32 i=0; i<Curve->Samples.size(); i++ )
		{
			Int32 MarkerX = Curve->Samples[i].Input * Size.Width + 5;
			if( abs(X-MarkerX) <= 5 )
				return i;
		}
	}

	// Nothing found.
	return -1;
}


//
// Redraw rainbow.
//
void WColorRibbon::UpdateRibbon()
{
	if( Curve )
	{
		// Very expensive calculations.
		TColor* Dest = (TColor*)Ribbon->GetData();
		for( Int32 u=0; u<Ribbon->USize; u++ )
			Dest[u] = Curve->SampleLinearAt( (Float)u / (Float)Ribbon->USize, COLOR_Black );
	}
	else
	{
		// Nothing to render.
		mem::zero( Ribbon->GetData(), Ribbon->GetBlockSize() );
	}

	Ribbon->bRedrawn = true;
}


//
// Redraw widget.
//
void WColorRibbon::OnPaint( CGUIRenderBase* Render )
{
	WWidget::OnPaint(Render);
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Ribbon itself.
	Render->DrawRegion
	(
		TPoint( Base.X, Base.Y ),
		TSize( Size.Width, Size.Height-17 ),
		TColor( 0x30, 0x30, 0x30, 0xff ),
		TColor( 0x30, 0x30, 0x30, 0xff ),
		BPAT_Solid
	);
	Render->DrawPicture
	(
		TPoint( Base.X+1, Base.Y+1 ),
		TSize( Size.Width-2, Size.Height-2-17 ),
		TPoint( 0, 0 ), 
		TSize( Ribbon->USize, Ribbon->VSize ),
		Ribbon
	);

	// Render markers.
	if( Curve )
	{
		for( Int32 i=0; i<Curve->Samples.size(); i++ )
		{
			auto Sample = Curve->Samples[i];

			// Triangle.
			Render->DrawPicture
			(
				TPoint( Base.X + Sample.Input*Size.Width, Base.Y + Size.Height-17 ),
				TSize( 11, 6 ),
				i == iSelected ? TPoint(11, 47) : TPoint(0, 47),
				TSize( 11, 6 ),
				WWindow::Icons
			);

			// Color.
			Render->DrawRegion
			(
				TPoint( Base.X + Sample.Input*Size.Width, Base.Y + Size.Height-11 ),
				TSize( 11, 11 ),
				Sample.Output,
				i == iSelected ? TColor( 0x87, 0x87, 0x87, 0xff ) : TColor( 0x30, 0x30, 0x30, 0xff ),
				BPAT_Solid
			);
		}
	}
}


//
// When ribbon changed.
//
void WColorRibbon::OnChange()
{
	EventChange(this);
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/