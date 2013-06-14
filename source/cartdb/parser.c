/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cartdb/parser.h"
#include "misc/log.h"

#define BUFSIZE	4096

static int num_malloc = 0;
static int num_malloc_str = 0;
static int num_malloc_node = 0;
static int num_malloc_attr = 0;
static int num_free = 0;
static int num_free_str = 0;
static int num_free_node = 0;
static int num_free_attr = 0;
size_t num_bytes = 0;

void *mem_malloc(size_t sz)
{
	num_malloc++;
	num_bytes += sz;
	return(malloc(sz));
}

void *mem_malloc_str(size_t sz)
{
	num_malloc_str++;
	return(mem_malloc(sz));
}

void *mem_malloc_node(size_t sz)
{
	num_malloc_node++;
	return(mem_malloc(sz));
}

void *mem_malloc_attr(size_t sz)
{
	num_malloc_attr++;
	return(mem_malloc(sz));
}

void mem_free(void *ptr)
{
	num_free++;
	free(ptr);
}

void mem_free_str(void *ptr)
{
	num_free_str++;
	mem_free(ptr);
}

void mem_free_node(void *ptr)
{
	num_free_node++;
	mem_free(ptr);
}

void mem_free_attr(void *ptr)
{
	num_free_attr++;
	mem_free(ptr);
}

static char *copystr(const char *str)
{
	char *ret;
	size_t sz;

	sz = strlen(str);
	ret = mem_malloc_str(sz + 1);
	memset(ret,0,sz + 1);
	memcpy(ret,str,sz);
	return(ret);
}

static node_t *create_node(const char *name)
{
	node_t *ret = 0;

	ret = (node_t*)mem_malloc_node(sizeof(node_t));
	memset(ret,0,sizeof(node_t));
	ret->name = copystr(name);
	return(ret);
}

static attribute_t *create_attribute(const char *name,const char *data)
{
	attribute_t *ret = 0;

	ret = (attribute_t*)mem_malloc_attr(sizeof(attribute_t));
	memset(ret,0,sizeof(attribute_t));
	ret->name = copystr(name);
	ret->data = copystr(data);
	return(ret);
}

static void destroy_node(node_t *node)
{
	mem_free_str(node->name);
	mem_free_node(node);
}

static void destroy_attribute(attribute_t *attr)
{
	mem_free_str(attr->name);
	mem_free_str(attr->data);
	mem_free_attr(attr);
}

static node_t *add_child(node_t *parent,const char *name)
{
	node_t *ret = 0;
	node_t *node;

	ret = create_node(name);
	ret->parent = parent;
	if(parent->child == 0) {
		parent->child = ret;
	}
	else {
		node = parent->child;
		while(node->next) {
			node = node->next;
		}
		node->next = ret;
	}
	return(ret);
}

static void free_attributes(attribute_t *attr)
{
	attribute_t *a;

	while(attr) {
		a = attr;
		attr = attr->next;
		destroy_attribute(a);
	}
}

static void free_nodes(node_t *node)
{
	node_t *n;

	while(node) {
		if(node->child)
			free_nodes(node->child);
		n = node;
		node = node->next;
		free_attributes(n->attributes);
		destroy_node(n);
	}
}

static void XMLCALL start(void *data,const char *el,const char **attr)
{
	xml_t *xmlinfo = (xml_t*)data;
	int i;

	if(xmlinfo->root == 0) {
		xmlinfo->root = create_node(el);
		xmlinfo->cur = xmlinfo->root;
	}
	else {
		xmlinfo->cur = add_child(xmlinfo->cur,el);
	}
	for(i=0;attr[i];i+=2) {
		attribute_t *at;

		at = create_attribute(attr[i],attr[i+1]);
		at->next = xmlinfo->cur->attributes;
		xmlinfo->cur->attributes = at;
	}
}

static void XMLCALL end(void *data,const char *el)
{
	xml_t *xmlinfo = (xml_t*)data;

	xmlinfo->cur = xmlinfo->cur->parent;
}

xml_t *parser_load(char *filename)
{
	FILE *fp;
	char *buf;
	int filelen,len;
	xml_t *xml = 0;

	//open xml file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("parser_loadxml:  error opening file '%s'\n",filename);
		return(0);
	}

	//get size of xml file
	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//allocate temporary buffer
	buf = mem_malloc(BUFSIZE + 1);

	//initialize xml struct
	xml = (xml_t*)mem_malloc(sizeof(xml_t));
	memset(xml,0,sizeof(xml_t));
	xml->parser = XML_ParserCreate(NULL);
	xml->root = xml->cur = 0;

	//setup expat
	XML_SetElementHandler(xml->parser,start,end);
	XML_SetUserData(xml->parser,(void*)xml);

	//help expat parse the xml file
	while(filelen) {
		if(BUFSIZE > filelen) {
			fread(buf,1,filelen,fp);
			len = filelen;
		}
		else {
			fread(buf,1,BUFSIZE,fp);
			len = BUFSIZE;
		}
		filelen -= len;
		XML_Parse(xml->parser,buf,len,0);
	}
	XML_Parse(xml->parser,"",0,1);

	//cleanup
	XML_ParserFree(xml->parser);
	xml->parser = 0;
	mem_free(buf);
	fclose(fp);

	//we are done
	return(xml);
}

void parser_free(xml_t *xml)
{
	free_nodes(xml->root);
	mem_free(xml);
}

void parser_merge(xml_t *dest,xml_t **src)
{
	node_t *node;

	//ensure we are working with the same types
	if(strcmp(dest->root->name,(*src)->root->name) != 0) {
		log_printf("parser_merge:  different root element names, cannot merge\n");
		return;
	}

	//find the last node
	node = dest->root->child;
	while(node->next) {
		node = node->next;
	}

	//move nodes to the dest xml struct
	node->next = (*src)->root->child;
	(*src)->root->child = 0;

	//free the rest of the src xml struct
	parser_free(*src);
	*src = 0;
}

#define IS_OK(cc)	((cc) ? "OK" : "Bad!")

void parser_verifymemory()
{
	int result;

	result = num_malloc_str - num_free_str;
	result += num_malloc_node - num_free_node;
	result += num_malloc_attr - num_free_attr;
	result += num_malloc - num_free;
	if(result) {
		log_printf("xml memory verification:\n");
		log_printf("------------------------\n");
		log_printf("  str mallocs:  %d, frees %d (%s)\n",num_malloc_str,num_free_str,IS_OK(num_malloc_str == num_free_str));
		log_printf("  node mallocs:  %d, frees %d (%s)\n",num_malloc_node,num_free_node,IS_OK(num_malloc_node == num_free_node));
		log_printf("  attr mallocs:  %d, frees %d (%s)\n",num_malloc_attr,num_free_attr,IS_OK(num_malloc_attr == num_free_attr));
		log_printf("  total mallocs:  %d, frees %d (%s)\n",num_malloc,num_free,IS_OK(num_malloc == num_free));
	}
	log_printf("total bytes used by xml:  %.3fmb\n",(double)num_bytes / 1024.0f / 1024.0f);
}
