const admin = require('firebase-admin');
const serviceAccount = require('../env/despro-canteen-ethics-firebase-adminsdk-27dsc-611de9fa59.json'); // Update this path

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  // storageBucket: 'gs://despro-project-monitoring.appspot.com', // Replace with your storage bucket
});

const db = admin.firestore();
// const bucket = admin.storage().bucket();

module.exports = { 
  db 
  // ,bucket 
};