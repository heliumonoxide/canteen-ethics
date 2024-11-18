// eslint-disable-next-line no-unused-vars
import React, { useEffect, useState } from 'react';

const AverageCard = () => {
  const [average, setAverage] = useState(null);
  const [error, setError] = useState('');

  useEffect(() => {
    const fetchAverage = async () => {
      try {
        // Assuming this is the correct endpoint
        const response = await fetch('https://canteen-ethics-api.vercel.app/api/average');
        if (!response.ok) throw new Error('Failed to fetch data');
        const data = await response.json();
        setAverage(data.average); // Assuming API response format is { average: <number> }
      } catch (err) {
        setError(err.message);
      }
    };

    fetchAverage();
  }, []);

  return (
    <div className="flex items-center justify-center">
      <div className="max-w-sm w-full bg-white shadow-lg rounded-lg overflow-hidden">
        <div className="bg-green-600 text-white py-4 px-6 text-center">
          <h2 className="text-xl font-bold font-inter">Average Violations (Last 7 Days)</h2>
        </div>
        <div className="px-6 py-3 flex flex-col items-center">
          {error ? (
            <p className="text-red-500 font-inter">{error}</p>
          ) : average !== null ? (
            <>
              <p className="text-2xl font-extrabold text-green-600 font-inter">{average}</p>
              <p className="text-lg font-semibold text-gray-700 font-inter">violations</p>
            </>
          ) : (
            <p className="text-gray-500 font-inter">Loading...</p>
          )}
        </div>
      </div>
    </div>
  );
};

export default AverageCard;
