// This file is part of snark, a generic and flexible library for robotics research
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef SNARK_ACTUATORS_UR_ROBOTIC_ARM_COMMANDS_H
#define SNARK_ACTUATORS_UR_ROBOTIC_ARM_COMMANDS_H

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <Eigen/Geometry>
#include <comma/base/types.h>
#include <comma/csv/ascii.h>
#include <comma/dispatch/dispatched.h>
#include "units.h"

namespace snark { namespace ur {

struct errors 
{ 
    enum error_code { success=0, unknown_command=401, format_error=402, incorrrect_operation=403 }; 
};

template < typename Derived > 
struct serialiser
{
    // Returns the same copy when called, doesnt build copy if not called
    static comma::csv::ascii< Derived >& ascii() { static comma::csv::ascii< Derived > ascii; return ascii; }
    const std::string& serialise() const { static std::string str; ascii().put( static_cast< const Derived& >( *this ), str ); return str; }
    const std::string& names() const { static std::string names = comma::join( comma::csv::names< Derived >() , ',' ); return names; }
};

template < typename Derived >
struct command_base : public serialiser< Derived >, public comma::dispatch::dispatched< Derived >
{
    comma::uint16 robot_id;         /// The robot's ID number
    comma::int32 sequence_number;   /// Command seuqence number
    std::string name;   /// Command name e.g. STEER
    command_base() : robot_id(0), sequence_number(0) {}
    command_base( comma::uint16 id, comma::int32 seq_no, const char* name_ ) : robot_id( id ), sequence_number( seq_no ), name( name_ ) {}    
};

struct move_joints : command_base< move_joints >
{
    move_joints() : joints( joint_num ) {}
    static const char joint_num = 6;
    static const unsigned char fields = 9; /// number of fields for command
    std::vector< plane_angle_degrees_t > joints; /// angle for each joint
};

/// For setting known pose: 'home' or 'giraffe'
struct set_position : command_base< set_position >
{
    set_position() : position( "home" ) {}    
    static const char fields = 4;
    std::string position;    
    enum { home=1, giraffe=2 };
};

struct auto_init_force : command_base< auto_init_force >
{
    static const char fields = 4;
    bool force;
};

/// Reads arm current joint positions and save as home position
struct set_home : command_base< set_home >
{
    set_home() : command_base< set_home >( 0, 0, "set_home" ) {}
};

struct position
{
    double x;
    double y;
    double z;
};

struct move_effector : command_base< move_effector >
{
    Eigen::Vector3d offset;
};

/// Send to trigger auto initialisation on startup.
struct auto_init : command_base< auto_init > {};

struct power : command_base< power > {
    bool is_on;
};

struct brakes : command_base< brakes > {
    bool enable;
};

/// Move an single joint at set speed in a given direction e.g. for initialisation.
struct joint_move : command_base< joint_move > {
    static const unsigned char fields = 5; /// number of fields for command
    comma::uint16 joint_id;
    bool dir;
};

} } // namespace snark { namespace ur {

#endif // SNARK_ACTUATORS_UR_ROBOTIC_ARM_COMMANDS_H
