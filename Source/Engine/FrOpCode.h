/*=============================================================================
	FrOpCode.h: List of OPerations CODEs.
	Created by Vlad Gordienko, Jul. 2016.
	Reimplemented by Vlad, Jan. 2018.
=============================================================================*/

//
// All codes.
//
enum EOpCode
{
	// Special marker 'End Of Code'.
	CODE_EOC,

	// Jump operations.
	CODE_Jump,
	CODE_JumpZero,
	CODE_Switch,
	CODE_Foreach,

	// L-value to R-value conversion.
	CODE_LToR,
	CODE_LToRDWord,
	CODE_LToRString,

	// Assignments.
	CODE_Assign	,
	CODE_AssignDWord,
	CODE_AssignString,

	// Variables loading.
	CODE_LocalVar,
	CODE_EntityProperty,
	CODE_BaseProperty,
	CODE_ComponentProperty,
	CODE_ResourceProperty,
	CODE_ProtoProperty,
	CODE_StaticProperty,

	// Constants.
	CODE_ConstByte,
	CODE_ConstBool,
	CODE_ConstInteger,
	CODE_ConstFloat,
	CODE_ConstAngle,
	CODE_ConstColor,
	CODE_ConstString,
	CODE_ConstVector,
	CODE_ConstAABB,
	CODE_ConstResource,
	CODE_ConstEntity,
	CODE_ConstDelegate,

	// Elements functions.
	CODE_ArrayElem,
	CODE_DynArrayElem,
	CODE_DynSetLength,
	CODE_DynGetLength,
	CODE_DynPush,
	CODE_DynPop,
	CODE_DynRemove,
	CODE_RMember,
	CODE_LMember,

	// Entity relative.
	CODE_This,
	CODE_New,
	CODE_Delete,
	CODE_EntityCast,
	CODE_FamilyCast,
	CODE_Is,
	CODE_In,
	CODE_Context,
	CODE_ThisContext,

	// Basic functions.
	CODE_Assert,
	CODE_Log,
	CODE_Length,

	// Comparison.
	CODE_Equal,
	CODE_NotEqual,

	// Thread operations.
	CODE_Label,
	CODE_Stop,
	CODE_Sleep,
	CODE_Wait,
	CODE_Goto,
	CODE_Interrupt,

	// Function call.
	CODE_BaseMethod,
	CODE_ComponentMethod,
	CODE_ResourceMethod,
	CODE_CallMethod,
	CODE_CallVF,
	CODE_CallDelegate,
	CODE_CallExtended,
	CODE_CallStatic,

	// Misc.
	CODE_VectorCnstr,
	CODE_ConditionalOp,
	CODE_DelegateCnstr,

	// Type cast.
	CAST_ByteToBool,
	CAST_ByteToInteger,
	CAST_ByteToFloat,
	CAST_ByteToString,
	CAST_BoolToInteger,
	CAST_BoolToString,
	CAST_IntegerToByte,
	CAST_IntegerToBool,
	CAST_IntegerToFloat,
	CAST_IntegerToAngle,
	CAST_IntegerToColor,
	CAST_IntegerToString,
	CAST_FloatToByte,
	CAST_FloatToBool,
	CAST_FloatToInteger,
	CAST_FloatToAngle,
	CAST_FloatToString,
	CAST_AngleToInteger,
	CAST_AngleToFloat,
	CAST_AngleToString,
	CAST_AngleToVector,
	CAST_ColorToInteger,
	CAST_ColorToString,
	CAST_StringToByte,
	CAST_StringToBool,
	CAST_StringToInteger,
	CAST_StringToFloat,
	CAST_VectorToBool,
	CAST_VectorToAngle,
	CAST_VectorToString,
	CAST_AabbToBool,
	CAST_AabbToStrnig,
	CAST_ResourceToBool,
	CAST_ResourceToString,
	CAST_EntityToBool,
	CAST_EntityToString,
	CAST_DelegateToBool,
	CAST_DelegateToString,

	// Unary operators.
	UN_Inc_Integer,
	UN_Inc_Float,
	UN_Dec_Integer,
	UN_Dec_Float,
	UN_Plus_Integer,
	UN_Plus_Float,
	UN_Plus_Vector,
	UN_Plus_Color,
	UN_Minus_Integer,
	UN_Minus_Float,
	UN_Minus_Vector,
	UN_Minus_Color,
	UN_Not_Bool,
	UN_Not_Integer,

	// Binary operators.
	BIN_Mult_Integer,
	BIN_Mult_Float,
	BIN_Mult_Color,
	BIN_Mult_Vector,
	BIN_Div_Integer,
	BIN_Div_Float,
	BIN_Mod_Integer,
	BIN_Add_Integer,
	BIN_Add_Float,
	BIN_Add_Color,
	BIN_Add_String,
	BIN_Add_Vector,
	BIN_Sub_Integer,
	BIN_Sub_Float,
	BIN_Sub_Color,
	BIN_Sub_Vector,
	BIN_Shr_Integer,
	BIN_Shl_Integer,
	BIN_Less_Integer,
	BIN_Less_Float,
	BIN_LessEq_Integer,
	BIN_LessEq_Float,
	BIN_Greater_Integer,
	BIN_Greater_Float,
	BIN_GreaterEq_Integer,
	BIN_GreaterEq_Float,
	BIN_And_Integer,
	BIN_Xor_Integer,
	BIN_Cross_Vector,
	BIN_Or_Integer,
	BIN_Dot_Vector,
	BIN_AddEqual_Integer,
	BIN_AddEqual_Float,
	BIN_AddEqual_Vector,
	BIN_AddEqual_String,
	BIN_AddEqual_Color,
	BIN_SubEqual_Integer,
	BIN_SubEqual_Float,
	BIN_SubEqual_Vector,
	BIN_SubEqual_Color,
	BIN_MulEqual_Integer,
	BIN_MulEqual_Float,
	BIN_MulEqual_Color,
	BIN_DivEqual_Integer,
	BIN_DivEqual_Float,
	BIN_ModEqual_Integer,
	BIN_ShlEqual_Integer,
	BIN_ShrEqual_Integer,
	BIN_AndEqual_Integer,
	BIN_XorEqual_Integer,
	BIN_OrEqual_Integer,

	// Native core functions.
	OP_Abs,
	OP_ArcTan,
	OP_ArcTan2,
	OP_Cos,
	OP_Sin,
	OP_Sqrt,
	OP_Distance,
	OP_Exp,
	OP_Ln,
	OP_Frac,
	OP_Round,
	OP_Normalize,
	OP_Random,
	OP_RandomF,
	OP_VectorSize,
	OP_RGBA,
	OP_CharAt,
	OP_IndexOf,
	OP_Execute,
	OP_Now,

	// Native engine functions.
	OP_PlaySoundFX,
	OP_PlayMusic,
	OP_KeyIsPressed,
	OP_GetScreenCursor,
	OP_GetWorldCursor,
	OP_Localize,
	OP_GetScript,
	OP_StaticPush,
	OP_StaticPop,
	OP_TravelTo,
	OP_FindEntity,
	OP_MatchKeyCombo,
	IT_AllEntities,
	IT_RectEntities,
	IT_TouchedEntities
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/