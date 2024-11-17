/* eslint-disable no-unused-vars */
// src/components/LineChart.jsx
import React, { useEffect, useState } from 'react';
import Chart from 'react-apexcharts';

const Graph = () => {
  const [chartData, setChartData] = useState({
    series: [{ name: 'Violations Sum', data: [] }],
    options: {
      colors: ['#28a745'], // Green theme
      chart: {
        height: 200,
        type: 'line',
        zoom: { enabled: true },
        offsetY: 0,
        toolbar: { show: true },
      },
      dataLabels: { enabled: false },
      stroke: {
        curve: 'smooth',
        colors: ['#28a745'], // Green stroke
        width: 2,
      },
      markers: {
        size: 5,
        colors: ['#28a745'], // Green markers
        strokeColors: '#ffffff',
        strokeWidth: 2,
        hover: { size: 7 },
      },
      xaxis: {
        categories: [], // This will hold the time_added values
        type: 'datetime',
        labels: {
          style: {
            fontSize: '12px',
            fontFamily: 'Inter, sans-serif',
            colors: '#333',
          },
        },
        title: {
          text: 'Time',
          style: {
            fontSize: '14px',
            fontWeight: 'bold',
            fontFamily: 'Inter, sans-serif',
            color: '#28a745',
          },
        },
      },
      title: {
        text: 'Plate left violation',
        align: 'left',
        style: {
          fontSize: '16px',
          fontWeight: 'bold',
          fontFamily: 'Inter, sans-serif',
          color: '#28a745',
        },
      },
      grid: {
        borderColor: '#e7e7e7',
        row: {
          colors: ['#f9f9f9', 'transparent'], // Alternating row colors
          opacity: 0.5,
        },
      },
      tooltip: {
        enabled: true,
        shared: true,
        intersect: false,
        x: {
          format: 'dd MMM yyyy HH:mm:ss',
          show: true,
        },
        y: {
          formatter: (val) => `${val} violations`,
        },
        marker: {
          show: true,
          colors: ['#28a745'], // Green tooltip marker
        },
      },
      legend: {
        show: true,
        position: 'top',
        horizontalAlign: 'center',
        labels: {
          colors: '#333',
          useSeriesColors: false,
          fontSize: '12px',
          fontFamily: 'Inter, sans-serif',
        },
      },
    },
  });

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch('http://localhost:5000/api/violation-result');
        const data = await response.json();

        // Ensure we have an array
        const results = Array.isArray(data) ? data : [];

        // Extract violation_sums and format time_added
        const violation_sums = results.map((doc) => doc.violation_sum);
        const time_added = results.map((doc) => {
          const utcDate = new Date(doc.time_added._seconds * 1000);
          const localDate = new Date(utcDate.setHours(utcDate.getHours() + 7));
          return localDate.toISOString();
        });

        // Update chart data
        setChartData((prevState) => ({
          ...prevState,
          series: [{ ...prevState.series[0], data: violation_sums }],
          options: {
            ...prevState.options,
            xaxis: { ...prevState.options.xaxis, categories: time_added },
          },
        }));
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchData();
  }, []);

  return (
    <div className="chart-container bg-white rounded-lg shadow-md p-2 w-full w-max-[100vw] min-w-[295px]">
      <Chart
        options={chartData.options}
        series={chartData.series}
        type="line"
        height={300}
      />
    </div>
  );
};

export default Graph;
