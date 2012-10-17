// This file is part of snark, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// snark is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// snark is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with snark. If not, see <http://www.gnu.org/licenses/>.

#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>
#include <comma/base/exception.h>
#include <comma/name_value/map.h>
#include <comma/string/string.h>
#include <snark/timing/ntp.h>
#include <snark/sensors/sick/protocol.h>

using namespace snark;

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "publish scan data from sick ld-mrs laser on stdout" << std::endl;
    std::cerr << "configure sick ld-mrs laser (sick ldmrs)" << std::endl;
    std::cerr << "todo: take ntp updates on stdin?" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage" << std::endl;
    std::cerr << "    sick-ldmrs-stream [<address:port>] [<options>]" << std::endl;
    std::cerr << "    todo? netcat ntp | sick-ldmrs-stream [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    <address:port>: ld-mrs laser address; default 192.168.0.1:12002" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: show this message" << std::endl;
    std::cerr << "    --get-status: get laser status" << std::endl;
    std::cerr << "    --get \"<name>[,<name>]\": output parameter values to stdout" << std::endl;
    std::cerr << "            address: get ip address" << std::endl;
    std::cerr << "            port: get tcp port" << std::endl;
    std::cerr << "    --reset: reset laser to factory settings and exit" << std::endl;
    std::cerr << "    --reset-dsp: reset laser dsp and exit" << std::endl;
    std::cerr << "    --set \"<name>=<value>[,<name>=<value>]\": set parameters and exit" << std::endl;
    std::cerr << "            address=<address>: set ip address" << std::endl;
    std::cerr << "            port=<port>: set tcp port" << std::endl;
    std::cerr << "    --start: start pumping laser scan data" << std::endl;
    std::cerr << "    --stop: stop pumping laser scan data" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << std::endl;
    std::cerr << "author:" << std::endl;
    std::cerr << "    vsevolod vlaskine, v.vlaskine@acfr.usyd.edu.au" << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

static bool verbose = false;
boost::scoped_ptr< sick::ldmrs::protocol > protocol;

static bool clear_fault()
{
    boost::optional< sick::ldmrs::fault > fault = protocol->last_fault();
    if( !fault ) { return true; }
    std::cerr << "sick-ldmrs-stream: " << *fault << std::endl;
    return !fault->fatal();
}

static void update_timestamp( bool force = false )
{
    static const boost::posix_time::time_duration timeout = boost::posix_time::seconds( 60 );
    static boost::posix_time::ptime last = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    if( !force && ( now - last ) < timeout ) { return; }
    if( verbose ) { std::cerr << "sick-ldmrs-stream: setting time to " << boost::posix_time::to_iso_string( now ) << "..." << std::endl; }
    std::pair< comma::uint32, comma::uint32 > ntp = snark::timing::to_ntp_time( now );
    clear_fault();
    if( !protocol->write( sick::ldmrs::commands::set_ntp_seconds( ntp.first ) ).ok() ) { COMMA_THROW( comma::exception, "failed to set NTP seconds" ); }
    clear_fault();
    if( !protocol->write( sick::ldmrs::commands::set_ntp_fractions( ntp.second ) ).ok() ) { COMMA_THROW( comma::exception, "failed to set NTP fractions" ); }
    last = now;
    if( verbose ) { std::cerr << "sick-ldmrs-stream: set time to " << boost::posix_time::to_iso_string( now ) << std::endl; }
}

int main( int ac, char** av )
{
    boost::scoped_ptr< boost::asio::ip::tcp::iostream > stream;
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        verbose = options.exists( "--verbose,-v" );
        std::vector< std::string > v = options.unnamed( "--help,-h,--get-status,--reset,--reset-dsp,--start,--stop,--verbose,-v", "--get,--set" );
        std::vector< std::string > a = comma::split( ( v.empty() ? std::string( "192.168.0.1:12002" ) : v[0] ), ':' );
        if( a.size() != 2 ) { std::cerr << "sick-ldmrs-stream: expected address, got \"" << v[0] << "\"" << std::endl; usage(); }
        std::string address = a[0];
        unsigned short port = boost::lexical_cast< unsigned short >( a[1] );
        boost::asio::io_service service;
        boost::asio::ip::tcp::resolver resolver( service );
        boost::asio::ip::tcp::resolver::query query( a[0], a[1] );
        boost::asio::ip::tcp::resolver::iterator it = resolver.resolve( query );
        if( verbose ) { std::cerr << "sick-ldmrs-stream: connecting to " << it->endpoint() << "..." << std::endl; }
        stream.reset( new boost::asio::ip::tcp::iostream( it->endpoint() ) );
        if( !( *stream ) ) { COMMA_THROW( comma::exception, "failed to connect to " << address << ":" << port ); }
        if( verbose ) { std::cerr << "sick-ldmrs-stream: connected to " << address << ":" << port << std::endl; }
        protocol.reset( new sick::ldmrs::protocol( *stream ) );
        options.assert_mutually_exclusive( "--get,--get-status,--reset,--set,--reset-dsp,--start,--stop" );
        bool ok = true;
        if( options.exists( "--reset" ) )
        {
            sick::ldmrs::commands::reset::response response = protocol->write( sick::ldmrs::commands::reset() );
            ok = response.ok();
            if( ok ) { if( verbose ) { std::cerr << "sick-ldmrs-stream: reset to factory settings; next time connect to the default ip address" << std::endl; } }
            clear_fault();
        }
        else if( options.exists( "--reset-dsp" ) )
        {
            protocol->reset_dsp(); 
            clear_fault();
        }
        else if( options.exists( "--set" ) )
        {
            comma::name_value::map m( options.value< std::string >( "--set" ), ',' );
            sick::ldmrs::commands::set command;
            for( comma::name_value::map::map_type::const_iterator it = m.get().begin(); ok && it != m.get().end(); ++it )
            {
                if( it->first == "address" )
                { 
                    command.index = sick::ldmrs::commands::set::ip_address;
                    std::vector< std::string > bytes = comma::split( it->second, '.' );
                    if( bytes.size() != 4 ) { COMMA_THROW( comma::exception, "expected ip address, got " << it->second ); }
                    command.value.data()[0] = boost::lexical_cast< unsigned int >( bytes[3] ); // little endian
                    command.value.data()[1] = boost::lexical_cast< unsigned int >( bytes[2] );
                    command.value.data()[2] = boost::lexical_cast< unsigned int >( bytes[1] );
                    command.value.data()[3] = boost::lexical_cast< unsigned int >( bytes[0] );
                }
                else if( it->first == "port" )
                {
                    command.index = sick::ldmrs::commands::set::tcp_port;
                    command.value = boost::lexical_cast< unsigned short >( it->second ); // todo: sort out endianness!
                }
                else
                { 
                    COMMA_THROW( comma::exception, "expected parameter, got " << it->first );
                }
                ok = protocol->write( command ).ok();
                if( !ok ) { std::cerr << "sick-ldmrs-stream: failed to set " << it->first << " to " << it->second << std::endl; }
                clear_fault();
            }
            if( ok )
            {
                ok = protocol->write( sick::ldmrs::commands::save_configuration() ).ok();
                if( !ok ) { std::cerr << "sick-ldmrs-stream: failed to save configuration" << std::endl; }
                else if( verbose ) { std::cerr << "sick-ldmrs-stream: saved configuration for " << options.value< std::string >( "--set" ) << std::endl; }
            }   
        }
        else if( options.exists( "--get" ) )
        {
            std::vector< std::string > v = comma::split( options.value< std::string >( "--get" ), ',' );
            sick::ldmrs::commands::get command;
            sick::ldmrs::commands::get::response response;
            std::string comma;
            for( std::size_t i = 0; ok && i < v.size(); ++i, ok = ok && response.ok() )
            {
                if( v[i] == "address" ) { command.index = sick::ldmrs::commands::set::ip_address; }
                else if( v[i] == "port" ) { command.index = sick::ldmrs::commands::set::tcp_port; }
                else { COMMA_THROW( comma::exception, "expected parameter, got " << v[i] ); }
                response = protocol->write( command );
                if( response.ok() ) { std::cout << comma << response; }
                comma = ",";
                clear_fault();
            }
        }
        else if( options.exists( "--get-status" ) )
        {
            sick::ldmrs::commands::get_status::response response = protocol->write( sick::ldmrs::commands::get_status() );
			ok = response.ok();
			if( ok ) { std::cout << response; }
            clear_fault();
        }
        else if( options.exists( "--start" ) )
        {
            ok = protocol->write( sick::ldmrs::commands::start() ).ok();
            clear_fault();
        }
        else if( options.exists( "--stop" ) )
        {
            ok = protocol->write( sick::ldmrs::commands::stop() ).ok();
            clear_fault();
        }
        else
        {
            update_timestamp( true );
            if( verbose ) { std::cerr << "sick-ldmrs-stream: starting scanning..." << std::endl; }
            if( !protocol->write( sick::ldmrs::commands::start() ).ok() ) { COMMA_THROW( comma::exception, "failed to start scanning" ); }
            if( verbose ) { std::cerr << "sick-ldmrs-stream: started scanning" << std::endl; }
            bool first = true;
            comma::signal_flag is_shutdown;
            while( !is_shutdown )
            {
                update_timestamp();
                clear_fault();
                const sick::ldmrs::scan_packet* scan;
                try { scan = protocol->readscan(); }
                catch( sick::ldmrs::protocol::faultException& ex ) { continue; }
                if( scan == NULL ) { break; }
                if( verbose && first ) { std::cerr << "sick-ldmrs-stream: got first scan" << std::endl; first = false; }
                if( !scan->packet_header.valid() ) { COMMA_THROW( comma::exception, "invalid scan" ); }
                std::cout.write( scan->data(), sick::ldmrs::header::size + scan->packet_header.payload_size() );
                if( verbose && ( scan->packet_scan.scan_header.measurement_number() % 10 == 0 ) )
                {
                    std::cerr << "sick-ldmrs-stream: got " << scan->packet_scan.scan_header.measurement_number() << " scans              \r";
                }
            }
            if( is_shutdown ) { std::cerr << "sick-ldmrs-stream: caught signal" << std::endl; }
        }
        if( ok ) { if( verbose ) { std::cerr << "sick-ldmrs-stream: done" << std::endl; } }
        else { std::cerr << "sick-ldmrs-stream: failed" << std::endl; }
        if( stream ) { stream->close(); }
        return ok ? 0 : -1;
    }
    catch( std::exception& ex )
    {
        std::cerr << "sick-ldmrs-stream: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "sick-ldmrs-stream: unknown exception" << std::endl;
    }
    if( stream ) { stream->close(); }
    usage();
}