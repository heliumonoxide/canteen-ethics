import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      // '/api': {
      //   target: 'https://webapi-two-flax.vercel.app',  // Backend server
      //   changeOrigin: true,
      //   secure: false,
      // rewrite: (path) => path.replace(/^\/api/, ''),  // Rewrite the URL to match your backend route
      // },
    },
  },
});
