import sys
import os
import pybind11
import mpi4py
from setuptools import setup, Extension
import setuptools.command.build_py
import sysconfig
import subprocess

setup(

      name='cfsil4py',
      version='1.0',
      description='Python bindings for FSI coupling utility of MUI coupling library.',
      url='',
      author='Wendi Liu',
      author_email='wendi.liu@stfc.ac.uk',
      license='GPL v3 & Apache v2',
      packages=['cfsil4py'],
      install_requires=[
            'mpi4py',
            'numpy',
       #     'mui4py',
      ],
      include_package_data=True,
      zip_safe=False)
