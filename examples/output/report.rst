Atmospheric Simulation
======================

A Brief++ feature tour
----------------------

:Author: Simulation Team
:Date: 28 September 2026
:Institution: Example Research Lab

.. rubric:: Abstract

A small, self-contained report showing the document model and all seven output formats.


.. _introduction:

Introduction and scope
----------------------

This report combines **measurements** with an *idealized* model. The word ``Brief++`` refers to the report builder. See the `project source <https://github.com/vrtulka23/briefpp>`_ for the code and :ref:`density-profile` for the generated figure. The report builder is documented by [example2026]_.

   A report should preserve the meaning of its content across formats.

.. note::

   The numbers below are illustrative rather than observational data.

.. _model:

Model
-----

The pressure model uses a scale height :math:`H = 8.4\,\mathrm{km}` under a constant-temperature assumption.

.. math::
   :label: hydrostatic

   \frac{dP}{dz} = -\rho g

.. math::
   :label: pressure-profile

   P(z) = P_0 e^{-z/H}

Equation :ref:`pressure-profile` describes the synthetic curve used here.

.. code-block:: cpp

   double pressure(double z, double p0, double h) {
       return p0 * std::exp(-z / h);
   }

.. _results:

Results
-------

.. _density-profile:

.. figure:: density.png
   :width: 80%

   Synthetic atmospheric **density** by altitude.

.. _conditions:

.. list-table:: Illustrative sea-level conditions
   :header-rows: 1

   * - Parameter
     - Value
     - Unit
   * - Temperature
     - 273.15
     - K
   * - Pressure
     - 101325
     - Pa
   * - **Scale height**
     - 8.4
     - km

The values in :ref:`conditions` produce the trend shown in :ref:`density-profile`.

1. Set the starting conditions
2. Review the output in every format
3. Compare the rendered documents

- **Check the details**
  - Check the figure and table
  - Check the linked references

Density
   Mass per unit volume

**Scale height**
   Altitude over which pressure falls by a factor of :math:`e`.

Interpretation
~~~~~~~~~~~~~~

The example emphasizes report structure, not atmospheric accuracy.

.. warning::

   Do not use these illustrative values for a real forecast.

.. tip::

   Edit this source file and rebuild the demo to compare formats.

----

.. list-table:: Headerless summary table
   :header-rows: 0

   * - Input
     - Illustrative profile
   * - Outputs
     - Seven text formats and a PDF

This paragraph was added from a ``DocumentFragment`` to demonstrate reusable content.

.. raw:: html

   <div class="page-break"></div>

Backend-specific notes
----------------------

.. rubric:: RST-only note

This line appears only in reStructuredText.

References
----------

.. [example2026] Brief++ contributors. Brief++ project source and documentation. 2026. `https://github.com/vrtulka23/briefpp <https://github.com/vrtulka23/briefpp>`_

