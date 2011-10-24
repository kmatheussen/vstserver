/*
    Copyright (C) 2002 Kjetil S. Matheussen / Notam.
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
    
*/

#ifndef VSTLIB_H
#define VSTLIB_H

#include <vst/AEffect.h>

/**
 * @brief Contacts the vstserver and try to load a plugin.
 *
 * @param filename can be an absolute or relative unix or windows path. It
 * does not need to contain a ".dll" suffix. Case is currently ignored.
 * If the filename is not an absolute path, the server first checks
 * in the directory the vstserver was started from, and
 * then inside the "VST_PATH" path.
 *
 * @return a pointer to a simulated VST AEffect object.
 *
 * @note You can not make a copy of this object with the memcpy (or eq.) function,
 * and use the copy.
 */
struct AEffect *VSTLIB_new(const char *filename);


/**
 *
 * @brief Reads cached AEffect information only.
 *
 * Does not contact the
 * server or load the plugin unless the plugin is not cached or
 * the cached information is outdated. This is a major speedup
 * if you just need to make a list of all plugins, which most programs
 * do.
 *
 * See file library/newdeletecache.c for a list of
 * currently supported opcodes. If an opcode is allready supported,
 * it will probably not be removed in the future.
 *
 * @return a pointer to a simulated VST AEffect object.
 *
 * @note The pointers to process, processreplace and setparameter in this
 * object are NULL, and must not be called.
 *
 * @note You can not make a copy of this object with the memcpy (or eq.) function,
 * and use the copy.
 */
struct AEffect *VSTLIB_newCache(const char *name);

/**
 * @brief Deletes an AEffect object allocated with VSTLIB_new or VSTLIB_newCache.
 */
void VSTLIB_delete(struct AEffect *effect);

/**
 * @brief Returns an array of cached AEffect objects for all plugins in VST_PATH.
 */
struct AEffect **VSTLIB_newCacheList(int *numberofplugins);

/**
 * @brief Delete the array of AEffect objects allocated with VSTLIB_newCacheList
 */
void VSTLIB_deleteCacheList(struct AEffect **aeffect);

/**
 * @brief Returns the filename used to load the plugin.
 *
 * Here is a program that print the names of all supposedly working plugins in the VST_PATH:
 *
 * \code
 * 
#include <stdio.h>
#include <vstlib.h>
int main(){
  int i,n;
  struct AEffect **effects=VSTLIB_newCacheList(&n);
  for(i=0;i<n;i++)
    puts(VSTLIB_getName(effects[i]));
  VSTLIB_deleteCacheList(effects);
  return 0;
}
\endcode
*/
const char *VSTLIB_getName(AEffect *effect);




#endif

