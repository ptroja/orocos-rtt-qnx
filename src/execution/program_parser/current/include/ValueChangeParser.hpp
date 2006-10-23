/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:26 CET 2004  ValueChangeParser.hpp

                        ValueChangeParser.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef VALUECHANGEPARSER_HPP
#define VALUECHANGEPARSER_HPP

#include "parser-types.hpp"

#include "Attribute.hpp"
#include "CommonParser.hpp"
#include "ExpressionParser.hpp"
#include "PeerParser.hpp"
#include "PropertyParser.hpp"
#include "Types.hpp"

namespace RTT { namespace detail
{
  /**
   * This class is responsible for parsing constant definitions,
   * variable definitions, variable change instructions, and alias
   * definitions..  It stores these in the ValueParser in the
   * ParseContext, and parses values using ExpressionParser..
   * @todo The ValueChangeParser.cxx implementation needs refactoring.
   */
  class ValueChangeParser
  {
      // all the AssignVariableCommand we've built..
      // This list is cleared in cleanup().
      std::vector<CommandInterface*> assigncommands;

      // the defined values...
      // This list is cleared in cleanup().
      std::vector<AttributeBase*> definedvalues;

      // the parsed variable or constant or alias or param
      // definition name.
      // This list is cleared in cleanup().
      std::vector<std::string> definednames;

      // A list of all the names stored in \a context.
      // this list is used to remove them when reset() is
      // called.
      std::vector<std::string> alldefinednames;

    // the name of the value of which we're currently parsing the
    // definition or assignment..
    std::string valuename;

    // A TypeInfo of the type that was specified.  We use it to get
    // hold of a Constant or a TaskVariable or ...
    TypeInfo* type;

    void seenconstantdefinition();
    void seenaliasdefinition();
    void seenvariabledefinition();
    void seenbaredefinition();
    void seenparamdefinition();
    void seenvariableassignment();
    void storedefinitionname( iter_t begin, iter_t end );
    void storename( iter_t begin, iter_t end );
    void storepeername();
    void seentype( iter_t begin, iter_t end );
    void seenindexassignment();
    void seensizehint();
    void seenproperty(); 

    rule_t constantdefinition, aliasdefinition, variabledefinition,
        variableassignment, variablechange, paramdefinition, baredefinition,
        vardecl, constdecl, baredecl;

    TaskContext* context;
    ExpressionParser expressionparser;
    PeerParser peerparser;
    PropertyParser propparser;
    CommonParser commonparser;

    DataSourceBase::shared_ptr index_ds;

      int sizehint;
      boost::shared_ptr<TypeInfoRepository> typerepos;

      /**
       * Delete temporary variables before throwing an exception.
       */
      void cleanup();
  public:
      /**
       * Create a ValueChangeParser which operates and stores values in a task.
       * Use definedvalues() to get the values added to \a tc, use store()
       * to store the added values in another task context as well. 
       * After reset(), \a tc will be cleared of all the stored values.
       * \a tc is thus used as a temporary storage container.
       */
      ValueChangeParser( TaskContext* tc);

      /**
       * Clear assignCommands(), definedValues() and 
       * definedNames(). Does not delete any variables or commands.
       */
      void clear();

      /**
       * Store allDefinedNames() in an additional TaskContext.
       */
      void store( TaskContext* other );

    /**
     * This CommandInterface holds the command assigning a value to
     * a variable that should be included in the program.  After a
     * constant definition, variable definition or variable
     * assignment is parsed, you should check it, and include it in
     * your program if it is non-zero.
     */
    CommandInterface* assignCommand()
      {
          if ( assigncommands.empty() )
              return 0;
          return assigncommands.back();
      }

    std::vector<CommandInterface*> assignCommands()
      {
          return assigncommands;
      }

    AttributeBase* lastDefinedValue()
      {
          if ( definedvalues.empty() )
              return 0;
          return definedvalues.back();
      }

    std::vector<AttributeBase*> definedValues()
      {
          return definedvalues;
      }

    std::string lastDefinedName()
      {
          if ( definednames.empty() )
              return "";
          return definednames.back();
      }

    std::vector<std::string> definedNames()
      {
          return definednames;
      }

    std::vector<std::string> allDefinedNames()
      {
          return alldefinednames;
      }


    /**
     * the parser that parses definitions of constants.  Do not
     * forget to check @ref assignCommand after a constant
     * definition is parsed..
     */
    rule_t& constantDefinitionParser();

    /**
     * the parser that parses variable definitions, don't forget to
     * check @ref assignCommand after a variable definition is
     * parsed..
     */
    rule_t& variableDefinitionParser();

    /**
     * the parser that parses variable assignments with 'set', don't forget to
     * check @ref assignCommand after a variable assignment is
     * parsed..
     */
    rule_t& variableAssignmentParser();

    /**
     * the parser that parses variable assignments without 'set' prefix , don't forget to
     * check @ref assignCommand after a variable assignment is
     * parsed..
     */
    rule_t& variableChangeParser();

    /**
     * The parser that parses alias definitions.  This does not work
     * via an assignment, and it is not necessary to check
     * assignCommand() after this..
     */
    rule_t& aliasDefinitionParser();

    /**
     * The parser that parses state context parameter definitions.
     * These do not get initialised where they are defined, so it is
     * not necessary to check assignCommand() after this...
     */
    rule_t& paramDefinitionParser();

    /**
     * The parser that parses a bare variable definition.
     * These do not get initialised where they are defined, so it is
     * not necessary to check assignCommand() after this...
     */
    rule_t& bareDefinitionParser();

    /**
     * Completely clear all data and erase all parsed
     * definitions from the taskcontext given in the constructor.
     */
    void reset();
  };
}}

#endif