/* EON - Canvas and Toolkit library
 * Copyright (C) 2008-2009 Jorge Luis Zapata
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EON_CONTAINER_H_
#define EON_CONTAINER_H_

typedef struct _Container_Private Container_Private;

struct _Container
{
	Object parent;
	Eina_Bool (*appendable)(const char *name);
	Container_Private *private;
};

Type *container_type_get(void);

EAPI void container_child_append(Container *p, Container *o);
EAPI void container_child_remove(Container *p, Container *o);

#endif /* EON_CONTAINER_H_ */
