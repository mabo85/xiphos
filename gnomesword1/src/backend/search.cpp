/*
 * GnomeSword Bible Study Tool
 * search.cpp - search Sword modules
 *
 * Copyright (C) 2000,2001,2002 GnomeSword Developer Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-1.2/glib.h>
#include <swmgr.h>
#include <markupfiltmgr.h>
#include <swmodule.h>
#include <swconfig.h>
#include <versekey.h>
#include <regex.h>
#include <pthread.h>

#include "backend/search.h"
#include "backend/sword_defs.h"

#include "main/settings.h"
#include "main/sword.h"
#include "main/search.h"

static VerseKey search_scope_bounds; //----------- sets lower and upper search bounds
static ListKey	search_scope_list; //----------- search list for searching verses found on last search
static SWKey	*current_scope; //----------- use to set scope of search	
static SWModule *search_module;
static ListKey search_range;



/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

char * backend_set_range(char * list)
{
	search_range = VerseKey().ParseVerseList(list, "", true);
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

GList * backend_get_element(char * entry)
{
	GList * rv = NULL;
	VerseKey key;
	
	ListKey verses = key.ParseVerseList(entry, key, true);
	for (int i = 0; i < verses.Count(); i++) {
		VerseKey *element = SWDYNAMIC_CAST(VerseKey, 
					verses.GetElement(i));
		if (element) {
			rv = g_list_append(rv,(char*)g_strdup((char *)
			(string(element->LowerBound()) + " - " + 
			string(element->UpperBound())).c_str()));
		}
		else	
			rv = g_list_append(rv,(char*)g_strdup(
				(const char *)*verses.GetElement(i)));
	}
	return rv;
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

void backend_save_custom_ranges(GList * ranges)
{
	GList *tmp = NULL;
	SWConfig *config = new SWConfig(settings.fnconfigure);	
	tmp = ranges;
	CUSTOM_RANGE *r;
	
	config->Sections["CustomRanges"].erase(
		config->Sections["CustomRanges"].begin(), 
		config->Sections["CustomRanges"].end());
	
	while(tmp != NULL) {
		r = (CUSTOM_RANGE*)tmp->data;
		config->Sections["CustomRanges"].insert(
			ConfigEntMap::value_type(r->label, r->range));
		tmp = g_list_next(tmp);
	}
	config->Save();
	delete config;
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

GList * backend_load_custom_ranges(void)
{
	CUSTOM_RANGE *r;
	GList * rv = NULL;
	ConfigEntMap::iterator loop, end;
	SWConfig *config = new SWConfig(settings.fnconfigure);	
	
	loop = config->Sections["CustomRanges"].begin();
	end = config->Sections["CustomRanges"].end();
	while (loop != end) {
		r = g_new(CUSTOM_RANGE,1);
		r->label = g_strdup(loop->first.c_str());
		r->range = g_strdup(loop->second.c_str());
		rv = g_list_append(rv, (CUSTOM_RANGE*)r);
		loop++;
	}
	delete config;	
	return rv;
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_clear_scope(void)
{
	current_scope = 0;	//------------ clear scope
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_clear_search_list(void)
{
	search_scope_list.ClearList();	//------------ clear scope search list
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_clear_bounds(void)
{
	search_scope_bounds.ClearBounds();
}

/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_set_bounds_upper(char * bound)
{
	search_scope_bounds.UpperBound(bound);
}

/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_set_bounds_lower(char * bound)
{
	search_scope_bounds.LowerBound(bound);
}

/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_set_scope2bounds(void)
{
	current_scope = &search_scope_bounds;
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_set_scope2last_search(void)
{
	current_scope = &search_scope_list;//-- if we do _ move searchlist into current_scope
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

void backend_set_scope2range(void)
{
	current_scope = &search_range;
}


/******************************************************************************
 * Name
 *   
 *
 * Synopsis
 *   #include "backend/search.h"
 *
 *   
 *
 * Description
 *   
 *
 * Return value
 *   
 */

int backend_set_search_module(char * mod_name)
{
	search_module = NULL;
	//g_warning(mod_name);
	search_module = sw_mgr.search->Modules[mod_name];
	if (search_module)
		return 1;
	else
		return 0;	
}


/******************************************************************************
 * Name
 *   backend_do_dialog_search
 *
 * Synopsis
 *   #include "search.h"
 *
 *   int backend_do_dialog_search(gpointer *search_opts)	
 *
 * Description
 *   search Sword module for word(s), phrase, or reg_ex.
 *
 * Return value
 *   int
 */
 
int backend_do_dialog_search(char *search_string, int search_type, int search_params)	
{	
	char progressunits = 70;
	int count = 0;		
	const char * result_text;
	strcpy(settings.searchText, search_string);
	
	if (search_module) {		/* must have a good module - not null */			  
		//-- give search string to sword for search
		for (ListKey & search_results = search_module->Search(
				search_string,
				search_type,
				search_params,
				current_scope, 0,
				&search_dialog_appbar_update,
				(void*)&progressunits);
				!search_results.Error ();
				search_results++) {		
			result_text = (const char *)search_results;	//-- put verse key string of find into a string			
			//g_warning((const char *)search_results);
					/* remember finds for next search's scope in case we want to use them */						
			search_scope_list << (const char *) search_results;
			add_to_found_list(result_text, search_module->Name());
			++count;	
                }
        } 
		
	return count;
}


/******************************************************************************
 * Name
 *   backend_do_sb_search
 *
 * Synopsis
 *   #include "search.h"
 *
 *   GList* backend_do_sb_search(gpointer *search_opts)	
 *
 * Description
 *   search Sword module for word(s), phrase, or reg_ex.
 *
 * Return value
 *   GList *
 */
 
GList* backend_do_sb_search(gpointer *usr_data)	
{	
	SEARCH_SWORD *search_opts;
	VerseKey searchScopeLowUp; //----------- sets lower and upper search bounds
        ListKey	searchScopeList; //----------- search list for searching verses found on last search
        SWKey	*currentScope; //----------- use to set scope of search	
	SWModule *searchMod,
			*tmpMod;
	ModMap::iterator 
		it;
			
	const char	
		*resultText;   //-- temp storage for verse found
	char     
		buf[256], 
		buf2[256], 
		*mybuf[2],
		scount[5],	//-- string from gint count for label
		firstkey[80];
	int            
		count;		//-- number of hits
	GList 
		*list;
	GString 
		*tmpbuf;
		
	search_opts = (SEARCH_SWORD *)usr_data;	
	tmpbuf = g_string_new("");	
	list = NULL;	
	
	
	searchMod = NULL;
	
	if(search_opts->module_name) {
		  it = sw_mgr.search->Modules.find (search_opts->module_name);
		  if (it != sw_mgr.search->Modules.end ()) {
			    searchMod = (*it).second;
			    tmpMod = (*it).second;
		  } else return list;
	} else return list;
	
	
	
	if (search_opts->use_bonds) {
		  searchScopeLowUp.ClearBounds ();	/* clear old bounds */
		  searchScopeLowUp.LowerBound (search_opts->lower_bond);	/* read lower bounds entry and set lower bounds for search */
		  searchScopeLowUp.UpperBound (search_opts->upper_bond);	/* read upper bounds entry and set upper bounds for search */
		  currentScope = &searchScopeLowUp;	/* set scope of search to use bounds */
	} 
	
	else if (search_opts->use_lastsearch_for_bonds) {	/* check to see if we want to use results of search last for this search */	  
		  currentScope = &searchScopeList;	//-- if we do _ move searchlist into currentScope
	} 
	
	else {
		  searchScopeLowUp.ClearBounds ();	//-------- clear old bounds
		  searchScopeList.ClearList ();	//------------ clear scope search list
		  currentScope = 0;	//------------ clear scope
	}
	
	char progressunits = 70;
	count = 0;		
	
	sprintf(settings.searchText, "%s",search_opts->search_string);
	
	if (searchMod) {		/* must have a good module - not null */	 
                int searchType = search_opts->search_type;
		int searchParams = search_opts->search_params;
			  
		//-- give search string to sword for search
		for (ListKey & searchResults = searchMod->Search (search_opts->search_string,
				searchType,
				searchParams,
				currentScope, 0,
				&search_percent_update,
				(void*)&progressunits);
				!searchResults.Error ();
				searchResults++) {		
			resultText = (const char *)searchResults;	//-- put verse key string of find into a string
			g_string_sprintf(tmpbuf,"%s, %s|%s|%s",
				resultText,
				searchMod->Name(),
				resultText,
				searchMod->Name());
			list = g_list_append(list,g_strdup(tmpbuf->str));
			/* remember finds for next search's scope in case we want to use them */						
			searchScopeList << (const char *) searchResults;	
			if(!count) 
				sprintf(firstkey,"%s",(const char *)searchResults);
			++count;	
                }
        } 
	search_opts->found_count = count;
		
	g_string_free(tmpbuf,TRUE);
	return list;
}

/******************************************************************************
 * Name
 *   backend_get_search_results_text
 *
 * Synopsis
 *   #include "search_.h"
 *
 *   char *backend_get_search_results_text(char * mod_name, char * key)	
 *
 * Description
 *    
 *
 * Return value
 *   char *
 */
 
char *backend_get_search_results_text(char * mod_name, char * key)
{
	SWModule *mod = sw_mgr.results->Modules[mod_name];
	
	if(mod) {
		mod->SetKey(key);
		return strdup((char*)mod->RenderText());
	}
	return NULL;
}
