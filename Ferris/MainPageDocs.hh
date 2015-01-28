/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: MainPageDocs.hh,v 1.2 2010/09/24 21:30:53 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


/*! \mainpage Ferris API docs
 *
 * \section intro Introduction
 *
 * Documentation is a tad scarce at this point in time. Most of the development
 * effort has been directed at making the core of ferris solid and creating a
 * set of nice contexts and attribute generators for its users.
 *
 * The two main abstractions in ferris are Attribute and Context. All data
 * is moved via IOStreams which are subclasses of std::iostreams but allow
 * streams to be passed by value to functions. Attribute and Context are
 * documented quite well at the moment, and if one is familiar with
 * std::iostreams then ferris iostreams have the same interface with some
 * enhancements. Note that passing a ferris iostream by value only copies
 * the handle, the underlying stream is not copied.
 *
 * A Context is like a directory or file in filesystem speak, and a Attribute
 * is like EA. Context objects make up a standard tree where each context
 * knows its parent and knows all of its direct children. Any number of
 * Attributes can be attached to a Context object and form another dimension
 * of information, much like having a bunch of files in a directory except
 * that attributes have a different namespace to Context objects attached as
 * children to a Context.
 *
 * \section latestdocs Getting the latest docs
 *
 * Note that tarballs of this documentation can be downloaded from
 * <A href="http://sourceforge.net/project/showfiles.php?group_id=16036">
 * sourceforge.net</A> for your offline enjoyment.
 *
 * Also by running doxygen in the main directory of a cvs checkout the latest
 * documentation will be generated in ../libferris.web/user-api/
 *
 * \section safeapi Safe API calls
 * 
 * There are some classes and functions exposed in the header files that are
 * for internal use only. They are currently exposed though their API may
 * change or they may be made redundant in any future versions. The reason
 * that these classes are exposed instead of a pure abstract class is
 * efficiency; to use implicit compiler casts the abstract interfaces would
 * need to virtually inherit from a superclass which would impose undue
 * complexity.
 *
 * In light of this, the safe classes and functions are explicitly listed here.
 * This list should be considered the authoritive listing.
 *
 * Ferris::Attribute, Ferris::Context, Ferris::Versioned, Ferris::CacheManager,
 * Ferris::ContextCollection, Ferris::MutableCollectionEvents,
 * Ferris::FerrisExceptionBase, Ferris::NamingEvent,
 * Ferris::AttributeCollection, Ferris::null_streambuf,
 * Ferris::ferris_basic_streambuf_sixteenk, Ferris::ferris_basic_streambuf_fourk,
 * Ferris::ferris_basic_streambuf_quartk, Ferris::ferris_basic_streambuf_sixteenbytes,
 * Ferris::ferris_basic_streambuf, Ferris::ferris_basic_double_buffered_streambuf,
 * Ferris::i_ferris_stream_traits, Ferris::o_ferris_stream_traits,
 * Ferris::io_ferris_stream_traits, Ferris::emptystream_methods,
 * Ferris::Ferris_commonstream, Ferris::Ferris_istream,
 * Ferris::Ferris_ostream, Ferris::Ferris_iostream,
 * Ferris::stringstream_methods, Ferris::ferris_stringbuf,
 * Ferris::Ferris_istringstream, Ferris::Ferris_ostringstream,
 * Ferris::Ferris_stringstream, Ferris::ferris_databuf,
 * Ferris::filestream_methods, Ferris::ferris_filebuf,
 * Ferris::Ferris_ifstream, Ferris::Ferris_ofstream
 * Ferris::Ferris_fstream
 * Ferris::fh_istream, Ferris::fh_ostream, Ferris::fh_iostream
 * Ferris::fh_istringstream, Ferris::fh_ostringstream, Ferris::fh_stringstream,
 * Ferris::fh_ifstream, Ferris::fh_ofstream, Ferris::fh_fstream, Ferris::
 * Ferris::fh_char
 * Ferris::Ferris_ififostream, Ferris::fh_ififostream,
 * 
 *
 * 
 * Note that stream inserters and extractors are assumed to be public
 *
 * Any of the exception types in FerrisException.hh may be used in a catch()
 * on their type explicitly.
 *
 * The following classes, only the methods listed in either the class itself or
 * and parent of that class that is listed above.
 *
 * Ferris::CreateMetaDataContext,
 *
 * The following functions
 *
 * Ferris::Resolve() <p>
 * Ferris::isBound() <p>
 * Ferris::tostr() <p>
 * Ferris::toint() <p>
 * Ferris::todouble() <p>
 * Ferris::StreamToString() <p>
 * Ferris::getFirstLine() <p>
 * Ferris::ends_with() <p>
 * Ferris::starts_with() <p>
 * Ferris::cmp_nocase() <p>
 * Ferris::getStrAttr() <p>
 * Ferris::getStrAttr() <p>
 * Ferris::getStrSubCtx() <p>
 * Ferris::isTrue() <p>
 * Ferris::getCacheManager() <p>
 * FERRIS_POPT_OPTIONS <p>
 * FERRIS_CONTEXTPOPT_OPTIONS <p>
 * <p>
 * namespace Ferris::Shell <p>
 * <p>
 * Ferris::Shell::getCWDString() <p>
 * Ferris::Shell::getCWD() <p>
 * Ferris::Shell::setCWD() <p>
 * Ferris::Shell::CreateFile() <p>
 * Ferris::Shell::CreateDir() <p>
 * <p>
 * namespace Ferris::Main <p>
 * <p>
 * Ferris::Main::EventPending() <p>
 * Ferris::Main::processEvent() <p>
 * Ferris::Main::mainLoop() <p>
 * <p>
 * <p>
 * namespace Ferris::Factory <p>
 * <p>
 * Ferris::Factory::Resolve() <p>
 * Ferris::Factory::ResolveMime() <p>
 * Ferris::Factory::ResolveIcon() <p>
 * Ferris::Factory::MakeSorter() <p>
 * Ferris::Factory::MakeFilter() <p>
 * Ferris::Factory::MakeFilterFromFile() <p>
 * Ferris::Factory::MakeFilteredContext() <p>
 * Ferris::Factory::MakeLimitingIStream() <p>
 * Ferris::Factory::MakeProxyStream() <p>
 * Ferris::Factory::MakeProxyStream() <p>
 * Ferris::Factory::MakeFdIStream() <p>
 * Ferris::Factory::MakeFdOStream() <p>
 * <p>
 * <p>
 * namespace Ferris::Math <p>
 * <p>
 * Ferris::Math::log() <p>
 * <p>
 * <p>
 * Use of other classes or functions may result in you having to change your code
 * later. If unsure if a function or class should be in the above list, please
 * send to the ferris-dev mailing list asking about it.
 *
 * \section qs Quick start
 *
 * Most things in ferris revolve around using a SmartPtr to an object.
 * Basically this allows the client to get an object and when the handle
 * to that object goes out of scope then the object may be freed by Ferris.
 *
 * The most basic pattern of a ferris client is to get a Context object and
 * then use it. Use the Ferris::Resolve method passing in a URL to the object
 * of interest and you obtain a Context object for that URL.
 * Note that once a Context is obtained then all of that Context's children can
 * be traversed recursively and the parent of any Context is available.
 * This allows one to use the Resolve method only for initial bootstrapping.
 *
 * \subsection usectx Using the context
 *
 * The main flavor of the API can be seen in Ferris.hh and Attribute.hh,
 * in particular the Attribute and Context classes. 
 *
 * An Attribute is a chunk of arbitrary size data attached
 * to a Context. At the moment there is no hierarchy of attributes in a context,
 * each attribute is just attached using a relative domain name (rdn). An rdn
 * is kind of like a file name, the two terms are used interchangeably in this
 * API reference.
 * Use Attribute::getAttribute(), Attribute::getAttributeNames() and
 * Attribute::getAttributeCount() to get at the attributes.
 *    
 *
 * Contexts may have sub contexts and a bunch of attributes attached to them.
 * There are some special attributes like "ea-names" that are always present and
 * are presented for completeness. By having these special attributes metadata
 * about the attributes can be presented along with the attributes themselves in
 * an GUI browser that uses ferris. See the ContextCollection class which is a
 * superclass of Context for methods dealing with access to the hierarchy of
 * Context objects.
 *
 * \section popt Popt interaction
 *
 * Simply include FERRIS_POPT_OPTIONS as a member in your popt arg table and
 * new --ferris-x options are added and parsed if persent for you. Note that
 * these options will never have short option names and will always begin
 * with --ferris to seperate them from your client's options.
 *
 *
 * \section ex Examples
 *
 * The <a href="http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/witme/libferris/tests/ls/ls.cpp?rev=HEAD&content-type=text/vnd.viewcvs-markup">ls.cpp</a> client provides a nice feel for client usage.
 *
 * There is also a Gtk+2 client for Ferris called "Ego". The Ego client is guile based and
 * presents the Ferris data via a TreeView.
 *
 *
 */
