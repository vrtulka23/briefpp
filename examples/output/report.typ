#set page(margin: 25mm)
= Atmospheric Simulation

_A Brief++ feature tour_

Simulation Team

28 September 2026

Example Research Lab

== Abstract

A small, self-contained report showing the document model and all seven output formats.

== Introduction and scope<introduction>

This report combines *measurements* with an _idealized_ model. The word #raw("Brief++") refers to the report builder. See the #link("https://github.com/vrtulka23/briefpp")[project source] for the code and @density-profile for the generated figure. The report builder is documented by #link(label("bib-example2026"))[\[example2026\]].

#quote(block: true)[A report should preserve the meaning of its content across formats.]

*note:* The numbers below are illustrative rather than observational data.

== Model<model>

The pressure model uses a scale height #raw("H = 8.4\\,\\mathrm{km}") under a constant-temperature assumption.

#raw("\\frac{dP}{dz} = -\\rho g", block: true)<hydrostatic>

$ P(z) = P_0 e^{-z/H} $<pressure-profile>

Equation @pressure-profile describes the synthetic curve used here.

#raw("double pressure(double z, double p0, double h) {\n    return p0 * std::exp(-z / h);\n}", block: true, lang: "cpp")

== Results<results>

#figure(image("density.png", width: 80%), caption: [Synthetic atmospheric *density* by altitude.])<density-profile>

#figure(table(columns: 3, table.header([Parameter], [Value], [Unit]), [Temperature], [273.15], [K], [Pressure], [101325], [Pa], [*Scale height*], [8.4], [km]), caption: [Illustrative sea-level conditions])<conditions>

The values in @conditions produce the trend shown in @density-profile.

+ Set the starting conditions
+ Review the output in every format
+ Compare the rendered documents

- *Check the details*
  - Check the figure and table
  - Check the linked references

/ Density: Mass per unit volume
/ *Scale height*: Altitude over which pressure falls by a factor of $e$.

=== Interpretation

The example emphasizes report structure, not atmospheric accuracy.

*warning:* Do not use these illustrative values for a real forecast.

*tip:* Edit this source file and rebuild the demo to compare formats.

#line(length: 100%)

#figure(table(columns: 2, [Input], [Illustrative profile], [Outputs], [Seven text formats and a PDF]), caption: [Headerless summary table])

This paragraph was added from a #raw("DocumentFragment") to demonstrate reusable content.

#pagebreak()

== Backend-specific notes

#align(center)[Typst-only note.]

== References

#block[\[example2026\] Brief++ contributors. Brief++ project source and documentation. 2026. #link("https://github.com/vrtulka23/briefpp")[https://github.com/vrtulka23/briefpp]]<bib-example2026>
