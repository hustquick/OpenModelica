within ThermoSysPro.InstrumentationAndControl.Blocks.Discret;
block ConvDA 
  parameter Real maxval=1 "Valeur maximale en entr�e";
  parameter Real minval=-maxval "Valeur minimale en entr�e";
  parameter Real bits=12 "Nombre de bits du convertisseur DA";
  parameter Real SampleOffset=0 "Instant de d�part de l'�chantillonnage (s)";
  parameter Real SampleInterval=0.01 "P�riode d'�chantillonnage (s)";
  
protected 
  Real qInterval(start=((maxval - minval)/2^bits)) "quantization interval";
  Real uBound "bounded input";
  annotation (
    Coordsys(
      extent=[-100, -100; 100, 100],
      grid=[2, 2],
      component=[20, 20]),
    Icon(
      Rectangle(extent=[-100, 100; 100, -100], style(fillColor=8)),
      Line(points=[-80, -30; -46, -30; -46, 0; -20, 0; -20, 22; -8, 22; -8, 44;
             12, 44; 12, 20; 30, 20; 30, 0; 62, 0; 62, -22; 88, -22]),
      Text(extent=[-150, 150; 150, 110], string="%name")),
    Diagram(Line(points=[-80, -30; -46, -30; -46, 0; -20, 0; -20, 22; -8, 22; -8,
             44; 12, 44; 12, 20; 30, 20; 30, 0; 62, 0; 62, -22; 88, -22])),
    Window(
      x=0.15,
      y=0.24,
      width=0.6,
      height=0.6),
    Documentation(info="<html>
<p><b>Version 1.0</b></p>
</HTML>
"));
public 
  ThermoSysPro.InstrumentationAndControl.Connectors.InputReal u 
                                      annotation (extent=[-120, -10; -100, 10]);
  ThermoSysPro.InstrumentationAndControl.Connectors.OutputReal y 
                                       annotation (extent=[100, -10; 120, 10]);
algorithm 
  
  qInterval := ((maxval - minval)/2^bits);
  
  when sample(SampleOffset, SampleInterval) then
    uBound := if u.signal > maxval then maxval else if u.signal < minval then 
      minval else u.signal;
    y.signal := qInterval*floor(abs(uBound/qInterval) + 0.5)*sign(uBound);
  end when;
end ConvDA;