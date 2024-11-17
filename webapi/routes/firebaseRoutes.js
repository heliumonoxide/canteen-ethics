const express = require('express');
const router = express.Router();
const firebaseController = require('../controllers/firebaseController');

// GET route to retrieve last 7 days results
router.get('/violation-result', firebaseController.getNewestSevenViolations);

// GET route to retrieve average sum results
router.get('/average', firebaseController.getAverageViolationSum);

module.exports = router;
