## 
# @file
# Python setup.py script to install Supersocket wrapper extension.
#
# @author Benjamin Shanahan

from distutils.core import setup, Extension
from sysconfig import get_config_var
from os import environ

include_dir = "../"

CFLAGS = "-DNOFPRINTF"

ext_sources = [

    "supersocket.i",

    "../Message.c",
    "../SocketWrapper.c",
    "../Supersocket.c",
    "../SupersocketListener.c",
    "../Display.c",
    "../ManageHeapMemory.c"

]

ext_modules = Extension(

    name         = "_supersocket",

    sources      = ext_sources,
    include_dirs = [ include_dir ]

)

config_var = get_config_var("CFLAGS")
# environ['CFLAGS'] = CFLAGS + " " + str(config_var)
# if "-Wstrict-prototypes" in config_var:
#     environ['CFLAGS'] = config_var.replace("-Wstrict-prototypes", "")
# if "-Wimplicit-function-declaration" in config_var:
#     environ['CFLAGS'] = config_var.replace("-Wimplicit-function-declaration", "")

setup(
    
    name            = "supersocket", 
    version         = "1.0",
    description     = "Supersocket Python Wrapper",
    author          = "Benjamin Shanahan",
    
    ext_modules = [ ext_modules ]

)