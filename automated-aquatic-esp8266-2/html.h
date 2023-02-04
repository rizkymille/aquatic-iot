const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>

  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <style>
      body {
        min-width: 310px;
        max-width: 800px;
        height: 400px;
        margin: 0 auto;
      }
      h2 {
        font-family: Arial;
        font-size: 2.5rem;
        text-align: center;
      }
    </style>
  </head>
  
  <body>
    <h2>Aquatic IoT Dashboard</h2>
    <div id="chart-temperature" class="container"></div>
    <div id="chart-pH" class="container"></div>
    <div id="chart-water-level" class="container"></div>
    <div id="chart-turbidity" class="container"></div>
  </body>

  <script>
  var chartT = new Highcharts.Chart({
    chart:{ renderTo : 'chart-temperature' },
    title: { text: 'Water Temperature' },
    series: [{
      showInLegend: false,
      data: []
    }],
    plotOptions: {
      line: { animation: false,
        dataLabels: { enabled: true }
      },
      series: { color: '#059e8a' }
    },
    xAxis: { type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { text: 'Temperature (Celsius)' }
      //title: { text: 'Temperature (Fahrenheit)' }
    },
    credits: { enabled: false }
  });
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var x = (new Date()).getTime(),
            y = parseFloat(this.responseText);
        //console.log(this.responseText);
        if(chartT.series[0].data.length > 40) {
          chartT.series[0].addPoint([x, y], true, true, true);
        } else {
          chartT.series[0].addPoint([x, y], true, false, true);
        }
      }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
  }, 30000 ) ;

  var chartPH = new Highcharts.Chart({
    chart:{ renderTo:'chart-pH' },
    title: { text: 'Water pH' },
    series: [{
      showInLegend: false,
      data: []
    }],
    plotOptions: {
      line: { animation: false,
        dataLabels: { enabled: true }
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { text: 'pH (0-14)' }
    },
    credits: { enabled: false }
  });
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var x = (new Date()).getTime(),
            y = parseFloat(this.responseText);
        //console.log(this.responseText);
        if(chartPH.series[0].data.length > 40) {
          chartPH.series[0].addPoint([x, y], true, true, true);
        } else {
          chartPH.series[0].addPoint([x, y], true, false, true);
        }
      }
    };
    xhttp.open("GET", "/ph", true);
    xhttp.send();
  }, 30000 ) ;

  var chartWL = new Highcharts.Chart({
    chart:{ renderTo:'chart-water-level' },
    title: { text: 'Aquatic Water Level' },
    series: [{
      showInLegend: false,
      data: []
    }],
    plotOptions: {
      line: { animation: false,
        dataLabels: { enabled: true }
      },
      series: { color: '#18009c' }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { text: 'Water Level (cm)' }
    },
    credits: { enabled: false }
  });
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var x = (new Date()).getTime(),
            y = parseFloat(this.responseText);
        //console.log(this.responseText);
        if(chartWL.series[0].data.length > 40) {
          chartWL.series[0].addPoint([x, y], true, true, true);
        } else {
          chartWL.series[0].addPoint([x, y], true, false, true);
        }
      }
    };
    xhttp.open("GET", "/water-level", true);
    xhttp.send();
  }, 30000 ) ;

  var chartTB = new Highcharts.Chart({
    chart:{ renderTo:'chart-turbidity' },
    title: { text: 'Water Turbidity' },
    series: [{
      showInLegend: false,
      data: []
    }],
    plotOptions: {
      line: { animation: false,
        dataLabels: { enabled: true }
      },
      series: { color: '#18009c' }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { text: 'Turbidity (NTU)' }
    },
    credits: { enabled: false }
  });
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var x = (new Date()).getTime(),
            y = parseFloat(this.responseText);
        //console.log(this.responseText);
        if(chartTB.series[0].data.length > 40) {
          chartTB.series[0].addPoint([x, y], true, true, true);
        } else {
          chartTB.series[0].addPoint([x, y], true, false, true);
        }
      }
    };
    xhttp.open("GET", "/turbidity", true);
    xhttp.send();
  }, 30000 ) ;
  </script>

  </html>
  )rawliteral";
