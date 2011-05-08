#!/usr/bin/python
#
# Routines to pull information out of YML config files:
#   - config/Version.yml
#   - config/Ports.yml
#   - config/Methods.yml
#
import yaml
import re
import subprocess
from itertools import chain

##
# A Python exception class for our use
##
class MadPackConfigError(Exception):
     def __init__(self, value):
         self.value = value
     def __str__(self):
         return repr(self.value)

## 
# Load version string from Version.yml file.
# Typical Version.yml file:
#       version: 0.01
# @param configdir the directory where we can find Version.yml
##
def get_version(configdir):

    try:
        conf = yaml.load(open(configdir + '/Version.yml'))
    except:
        print "configyml : ERROR : missing or malformed Version.yml"
        exit(2)

    try:
        conf['version']
    except:
        print "configyml : ERROR : malformed Version.yml"
        exit(2)
        
    return str( conf['version'])

## 
# Load Install.yml file.
# @param configdir the directory where we can find Version.yml
##
#def get_method_spec( methoddir):
#
#    try:
#        install = yaml.load(open(methoddir + '/Install.yml'))
#    except:
#        print "configyml : ERROR : missing or malformed Install.yml"
#        exit(2)
#
#    try:
#        install['module']
#    except:
#        print "configyml : ERROR : missing 'module' section in Install.yml"
#        exit(2)
#
#    try:
#        install['create']
#    except:
#        print "configyml : ERROR : missing 'create' section in Install.yml"
#        exit(2)
#        
#    return install
    
## 
# Load Ports.yml file
# @param configdir the directory where we can find Version.yml
##
def get_ports(configdir):

    try:
        conf = yaml.load(open(configdir + '/Ports.yml'))
    except:
        print "configyml : ERROR : missing or malformed Ports.yml"
        exit(2)

    try:
        conf['ports']
    except:
        print "configyml : ERROR : malformed Ports.yml"
        exit(2)
        
    for port in conf['ports']:
        try:
            port['name']
        except:
            print "configyml : ERROR : malformed Ports.yml: missing name element"
            exit(2)
        #try:
        #    port['src']
        #except:
        #    print "configyml : ERROR : malformed Ports.yml: no SRC element for port " + port['name']
        #    exit(2)
        try:
            port['id']
        except:
            print "configyml : ERROR : malformed Ports.yml: no ID element for port " + port['name']
            exit(2)
        try:
            port['dbapi2']
        except:
            print "configyml : ERROR : malformed Ports.yml: no DBAPI2 element for port " + port['name']
            exit(2)
        
    return conf
    
## 
# Load methods
#
# @param configdir the directory where we can find the [port_id].yml file
# @param id the ID of the specific DB port
# @param src the directory of the source code for a specific port
##
def get_methods( confdir):

    fname = "Methods.yml"
    
    try:
        conf = yaml.load( open( confdir + '/' + fname))
    except:
        print "configyml : ERROR : missing or malformed " + confdir + '/' + fname
        raise Exception

    #try:
    #    conf['env']
    #except:
    #    print "configyml : ERROR : missing env section in " + fname
    #    exit(2)

    try:
        conf['methods']
    except:
        print "configyml : ERROR : missing methods section in " + fname
        raise Exception
        
    conf = topsort_methods( conf)
    
    return conf

##
# Helper function
##    
def flatten(listOfLists):
    "Flatten one level of nesting"
    return chain.from_iterable(listOfLists)

## 
# Quick and dirty topological sort
# Currently does dumb cycle detection.
# @param depdict an edgelist dictionary, e.g. {'b': ['a'], 'z': ['m', 'n'], 'm': ['a', 'b']}
##
def topsort(depdict):
    out = dict()
    candidates = set()
    curlevel = 0

    while len(depdict) > 0:
        found = 0  # flag to check if we find anything new this iteration
        newdepdict = dict()
        # find the keys with no values
        keynoval = filter(lambda t: t[1] == [], depdict.iteritems())
        # find the values that are not keys
        valnotkey = set(flatten(depdict.itervalues())) - set(depdict.iterkeys())

        candidates = set([k[0] for k in keynoval]) | valnotkey
        for c in candidates:
            if c not in out:
                found += 1
                out[c] = curlevel

        for k in depdict.iterkeys():
            if depdict[k] != []:
                newdepdict[k] = filter(lambda v: v not in valnotkey, depdict[k])
        # newdepdict = dict(newdepdict)
        if newdepdict == depdict:
            raise MadPackConfigError(str(depdict))
        else:
            depdict = newdepdict
        if found > 0:
            curlevel += 1
    
    return out

## 
# Top-sort the methods in conf
# @param conf a madpack configuration
##
def topsort_methods(conf):

    depdict = dict()    
    for m in conf['methods']:
        try:
            depdict[m['name']] = m['depends']
        except:
            depdict[m['name']] = []        
    try:
        method_dict = topsort(depdict)
    except MadPackConfigError as e:
        raise MadPackConfigError("invalid cyclic dependency between methods: " + e.value + "; check Methods.yml files")
    missing = set(method_dict.keys()) - set(depdict.keys())
    inverted = dict()
    if len(missing) > 0:
        for k in depdict.iterkeys():
            for v in depdict[k]:
                if v not in inverted:
                    inverted[v] = set()
                inverted[v].add(k)
        print "configyml : ERROR : required methods missing from Methods.yml: " 
        for m in missing:
            print  "    " + m + " (required by " + str(list(inverted[m])) + ")"
        exit(2)
    conf['methods'] = sorted(conf['methods'], key=lambda m:method_dict[m['name']])
    return conf