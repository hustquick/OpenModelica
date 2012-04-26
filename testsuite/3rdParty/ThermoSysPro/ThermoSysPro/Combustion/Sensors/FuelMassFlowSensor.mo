within ThermoSysPro.Combustion.Sensors;
model FuelMassFlowSensor "Fuel mass flow rate sensor" 
  
public 
  Modelica.SIunits.MassFlowRate Q(start=20) "Mass flow rate";
  
public 
  ThermoSysPro.InstrumentationAndControl.Connectors.OutputReal Mesure 
    annotation (extent=[-10,92; 10,112],    rotation=90);
  ThermoSysPro.Combustion.Connectors.FuelInlet C1 
    annotation (extent=[-110,-90; -90,-70]);
  ThermoSysPro.Combustion.Connectors.FuelOutlet C2 
    annotation (extent=[92,-90; 112,-70]);
equation 
  
  C1.Q = C2.Q;
  C1.T = C2.T;
  C1.P = C2.P;
  C1.LHV = C2.LHV;
  C1.cp = C2.cp;
  C1.hum = C2.hum;
  C1.Xc = C2.Xc;
  C1.Xh = C2.Xh;
  C1.Xo = C2.Xo;
  C1.Xn = C2.Xn;
  C1.Xs = C2.Xs;
  C1.Xashes = C2.Xashes;
  C1.VolM = C2.VolM;
  C1.rho = C2.rho;
  
  Q = C1.Q;
  
  /* Sensor signal */
  Mesure.signal = Q;
  
  annotation (
    Coordsys(
      extent=[-100, -100; 100, 100],
      grid=[2, 2],
      component=[20, 20]),
    Icon(
      Line(points=[0,-28; 0,-80]),
      Line(points=[-98, -80; 102, -80]),
      Ellipse(extent=[-60,92; 60,-28], style(
          fillColor=2,
          rgbfillColor={0,255,0},
          fillPattern=10)),
      Text(extent=[-60,60; 60,0],    string="Q")),
    Window(
      x=0.25,
      y=0.19,
      width=0.6,
      height=0.6),
    Diagram(
      Line(points=[0,-28; 0,-80], style(
          color=3,
          rgbcolor={0,0,255},
          fillColor=42,
          rgbfillColor={127,0,0})),
      Line(points=[-98, -80; 102, -80]),
      Ellipse(extent=[-60,92; 60,-28], style(
          fillColor=2,
          rgbfillColor={0,255,0},
          fillPattern=10)),
      Text(extent=[-60,60; 60,0],    string="Q")),
    Documentation(info="<html>
<p><b>Copyright &copy; EDF 2002 - 2010</b></p>
</HTML>
<html>
<p><b>ThermoSysPro Version 2.0</b></p>
</HTML>
", revisions="<html>
<u><p><b>Authors</u> : </p></b>
<ul style='margin-top:0cm' type=disc>
<li>
    Salimou Gassama</li>
</ul>
</html>
"));
end FuelMassFlowSensor;