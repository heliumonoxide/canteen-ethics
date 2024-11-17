// src/components/Dashboard.jsx
// eslint-disable-next-line no-unused-vars
import React from 'react';
import Graph from '../Graph/Graph';
import AverageCard from '../AverageCard/AverageCard';

const Dashboard = () => {
  return (
    <div className="min-h-screen bg-gray-50 text-gray-800 flex flex-col font-inter">
      {/* Header */}
      <header className="bg-green-600 p-6 shadow-md text-white">
        <h1 className="text-3xl font-semibold">Ethical Monitoring Canteen</h1>
        <p className="text-sm text-green-200">Promoting ethical behavior, one step at a time.</p>
      </header>

      {/* Main Content */}
      <main className="flex-1 flex flex-col items-center justify-center px-4 gap-10 py-5">
        <div className='flex flex-row flex-wrap justify-center gap-5'>
            <div className="w-full max-w-3xl bg-white shadow-md rounded-lg p-6 text-center">
            <h2 className="text-2xl font-bold text-green-700 mb-4">Welcome to the Dashboard</h2>
            <p className="text-gray-600">
                Here, you can monitor activities and ensure ethical standards are met within the canteen. Explore the
                features and tools available to make better decisions.
            </p>
            </div>
            <AverageCard />
        </div>
        <Graph />
      </main>

      {/* Footer */}
      <footer className="bg-green-600 text-white p-4 text-center">
        <p className="text-sm">&copy; {new Date().getFullYear()} Universitas Indonesia</p>
      </footer>
    </div>
  );
};

export default Dashboard;
