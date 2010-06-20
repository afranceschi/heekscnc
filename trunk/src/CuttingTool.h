
// CuttingTool.h
/*
 * Copyright (c) 2009, Dan Heeks, Perttu Ahola
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#pragma once

#include "Program.h"
#include "Op.h"
#include "HeeksCNCTypes.h"

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

#include <vector>
#include <algorithm>

class CCuttingTool;

class CCuttingToolParams{

public:

	typedef enum {
		eDrill = 0,
		eCentreDrill,
		eEndmill,
		eSlotCutter,
		eBallEndMill,
		eChamfer,
		eTurningTool,
		eTouchProbe,
		eToolLengthSwitch,
		eUndefinedToolType
	} eCuttingToolType;

	typedef std::pair< eCuttingToolType, wxString > CuttingToolTypeDescription_t;
	typedef std::vector<CuttingToolTypeDescription_t > CuttingToolTypesList_t;

	static CuttingToolTypesList_t GetCuttingToolTypesList()
	{
		CuttingToolTypesList_t types_list;

		types_list.push_back( CuttingToolTypeDescription_t( eDrill, wxString(_("Drill Bit")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eCentreDrill, wxString(_("Centre Drill Bit")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eEndmill, wxString(_("End Mill")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eSlotCutter, wxString(_("Slot Cutter")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eBallEndMill, wxString(_("Ball End Mill")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eChamfer, wxString(_("Chamfer")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eTurningTool, wxString(_("Turning Tool")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eTouchProbe, wxString(_("Touch Probe")) ));
		types_list.push_back( CuttingToolTypeDescription_t( eToolLengthSwitch, wxString(_("Tool Length Switch")) ));

		return(types_list);
	} // End GetCuttingToolTypesList() method



	typedef enum {
		eHighSpeedSteel = 0,
		eCarbide,
		eUndefinedMaterialType
	} eMaterial_t;

	typedef std::pair< eMaterial_t, wxString > MaterialDescription_t;
	typedef std::vector<MaterialDescription_t > MaterialsList_t;

	static MaterialsList_t GetMaterialsList()
	{
		MaterialsList_t materials_list;

		materials_list.push_back( MaterialDescription_t( eHighSpeedSteel, wxString(_("High Speed Steel")) ));
		materials_list.push_back( MaterialDescription_t( eCarbide, wxString(_("Carbide")) ));

		return(materials_list);
	} // End Get() method

	// The G10 command can be used (within EMC2) to add a tool to the tool
	// table from within a program.
	// G10 L1 P[tool number] R[radius] X[offset] Z[offset] Q[orientation]

	int m_material;	// eMaterial_t - describes the cutting surface type.

	double m_diameter;
	double m_tool_length_offset;

	// The following are all for lathe tools.  They become relevant when the m_type = eTurningTool
	double m_x_offset;
	double m_front_angle;
	double m_tool_angle;
	double m_back_angle;
	int m_orientation;
	// also m_corner_radius, see below, is used for turning tools and milling tools


	/**
		The next three parameters describe the cutting surfaces of the bit.

		The two radii go from the centre of the bit -> flat radius -> corner radius.
		The vertical_cutting_edge_angle is the angle between the centre line of the
		milling bit and the angle of the outside cutting edges.  For an end-mill, this
		would be zero.  i.e. the cutting edges are parallel to the centre line
		of the milling bit.  For a chamfering bit, it may be something like 45 degrees.
		i.e. 45 degrees from the centre line which has both cutting edges at 2 * 45 = 90
		degrees to each other

		For a ball-nose milling bit we would have;
			- m_corner_radius = m_diameter / 2
			- m_flat_radius = 0;	// No middle bit at the bottom of the cutter that remains flat
						// before the corner radius starts.
			- m_vertical_cutting_edge_angle = 0

		For an end-mill we would have;
			- m_corner_radius = 0;
			- m_flat_radius = m_diameter / 2
			- m_vertical_cutting_edge_angle = 0

		For a chamfering bit we would have;
			- m_corner_radius = 0;
			- m_flat_radius = 0;	// sharp pointed end.  This may be larger if we can't use the centre point.
			- m_vertical_cutting_edge_angle = 45	// degrees from centre line of tool
	 */
	double m_corner_radius;
	double m_flat_radius;
	double m_cutting_edge_angle;
	double m_cutting_edge_height;	// How far, from the bottom of the cutter, do the flutes extend?

	eCuttingToolType	m_type;
	double m_max_advance_per_revolution;	// This is the maximum distance a tool should advance during a single
						// revolution.  This value is often defined by the manufacturer in
						// terms of an advance no a per-tooth basis.  This value, however,
						// must be expressed on a per-revolution basis.  i.e. we don't want
						// to maintain the number of cutting teeth so a per-revolution
						// value is easier to use.

	int m_automatically_generate_title;	// Set to true by default but reset to false when the user edits the title.

	// The following coordinates relate ONLY to touch probe tools.  They describe
	// the error the probe tool has in locating an X,Y point.  These values are
	// added to a probed point's location to find the actual point.  The values
	// should come from calibrating the touch probe.  i.e. set machine position
	// to (0,0,0), drill a hole and then probe for the centre of the hole.  The
	// coordinates found by the centre finding operation should be entered into
	// these values verbatim.  These will represent how far off concentric the
	// touch probe's tip is with respect to the quil.  Of course, these only
	// make sense if the probe's body is aligned consistently each time.  I will
	// ASSUME this is correct.

	double m_probe_offset_x;
	double m_probe_offset_y;

	// The gradient is the steepest angle at which this tool can plunge into the material.  Many
	// cutting tools behave better if they are slowly ramped down into the material.  This gradient
	// specifies the steepest angle of decsent.  This is expected to be a negative number indicating
	// the 'rise / run' ratio.  Since the 'rise' will be downward, it will be negative.
	// By this measurement, a drill bit's straight plunge would have an infinite gradient (all rise, no run).
	// To cater for this, a value of zero will indicate a straight plunge.

	double m_gradient;

	void set_initial_values();
	void write_values_to_config();
	void GetProperties(CCuttingTool* parent, std::list<Property *> *list);
	void WriteXMLAttributes(TiXmlNode* pElem);
	void ReadParametersFromXMLElement(TiXmlElement* pElem);

	const wxString ConfigScope(void)const{return _T("CuttingToolParam_");}
	double ReasonableGradient( const eCuttingToolType type ) const;

	bool operator== ( const CCuttingToolParams & rhs ) const;
	bool operator!= ( const CCuttingToolParams & rhs ) const { return(! (*this == rhs)); }
};

class CCuttingTool: public HeeksObj {
public:
	//	These are references to the CAD elements whose position indicate where the CuttingTool Cycle begins.
	CCuttingToolParams m_params;
	wxString m_title;

	typedef int ToolNumber_t;
	ToolNumber_t m_tool_number;
	HeeksObj *m_pToolSolid;

	//	Constructors.
	CCuttingTool(const wxChar *title, CCuttingToolParams::eCuttingToolType type, const int tool_number) : m_tool_number(tool_number), m_pToolSolid(NULL)
	{
		m_params.set_initial_values();
		m_params.m_type = type;
		if (title != NULL)
		{
			m_title = title;
		} // End if - then
		else
		{
			m_title = GenerateMeaningfulName();
		} // End if - else
	} // End constructor

    CCuttingTool( const CCuttingTool & rhs );
    CCuttingTool & operator= ( const CCuttingTool & rhs );

	~CCuttingTool();

	bool operator== ( const CCuttingTool & rhs ) const;
	bool operator!= ( const CCuttingTool & rhs ) const { return(! (*this == rhs)); }

	bool IsDifferent( HeeksObj *other ) { return(*this != (*(CCuttingTool *)other)); }

	 // HeeksObj's virtual functions
        int GetType()const{return CuttingToolType;}
	const wxChar* GetTypeString(void) const{ return _T("CuttingTool"); }
        HeeksObj *MakeACopy(void)const;

        void WriteXML(TiXmlNode *root);
        static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	// program whose job is to generate RS-274 GCode.
	Python AppendTextToProgram();

	void GetProperties(std::list<Property *> *list);
	void CopyFrom(const HeeksObj* object);
	bool CanAddTo(HeeksObj* owner);
	const wxBitmap &GetIcon();
    const wxChar* GetShortString(void)const{return m_title.c_str();}
	void glCommands(bool select, bool marked, bool no_color);
	void KillGLLists(void);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);

        bool CanEditString(void)const{return true;}
        void OnEditString(const wxChar* str);

	static CCuttingTool *Find( const int tool_number );
	static int FindCuttingTool( const int tool_number );
	static ToolNumber_t FindFirstByType( const CCuttingToolParams::eCuttingToolType type );
	static std::vector< std::pair< int, wxString > > FindAllCuttingTools();
	wxString GenerateMeaningfulName() const;
	wxString ResetTitle();
	static wxString FractionalRepresentation( const double original_value, const int max_denominator = 64 );
	static wxString GuageNumberRepresentation( const double size, const double units );

	TopoDS_Shape GetShape() const;
	TopoDS_Face  GetSideProfile() const;

	double CuttingRadius(const bool express_in_drawing_units = false, const double depth = -1) const;
	static CCuttingToolParams::eCuttingToolType CutterType( const int tool_number );
	static CCuttingToolParams::eMaterial_t CutterMaterial( const int tool_number );

	void SetDiameter( const double diameter );
	void ResetParametersToReasonableValues();
	void ImportProbeCalibrationData( const wxString & probed_points_xml_file_name );
	double Gradient() const { return(m_params.m_gradient); }

private:
	void DeleteSolid();

}; // End CCuttingTool class definition.

