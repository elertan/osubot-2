#include "stdafx.h"
#include "HitObject.h"

namespace osu {
	bool HitObject::IsCircle() const
	{
		return (type & HITOBJECT_CIRCLE) == HITOBJECT_CIRCLE;
	}
	bool HitObject::IsSlider() const
	{
		return (type & HITOBJECT_SLIDER) == HITOBJECT_SLIDER;
	}
	bool HitObject::IsSpinner() const
	{
		return (type & HITOBJECT_SPINNER) == HITOBJECT_SPINNER;
	}
}