.. _gdal_raster_astype_subcommand:

================================================================================
"gdal raster astype" sub-command
================================================================================

.. versionadded:: 3.11

.. only:: html

    Modify the data type of bands of a raster dataset.

.. Index:: gdal raster astype

Synopsis
--------

.. program-output:: gdal raster astype --help-doc

Description
-----------

:program:`gdal raster astype` can be used to force the output image bands to
have a specific data type. Values may be truncated or rounded if the output
data type is "smaller" than the input one.

This subcommand is also available as a potential step of :ref:`gdal_raster_pipeline_subcommand`

Standard options
++++++++++++++++

.. include:: gdal_options/of_raster_create_copy.rst

.. include:: gdal_options/co.rst

.. include:: gdal_options/overwrite.rst

.. option:: --ot, --datatype, --output-data-type <OUTPUT-DATA-TYPE>

  Output data type among ``Byte``, ``Int8``, ``UInt16``, ``Int16``, ``UInt32``,
  ``Int32``, ``UInt64``, ``Int64``, ``CInt16``, ``CInt32``, ``Float32``,
  ``Float64``, ``CFloat32``, ``CFloat64``.

.. GDALG output (on-the-fly / streamed dataset)
.. --------------------------------------------

.. include:: gdal_cli_include/gdalg_raster_compatible.rst

Examples
--------

.. example::
   :title: Convert to Float32 data type

   .. code-block:: bash

        $ gdal raster astype --datatype Float32 byte.tif float32.tif --overwrite
