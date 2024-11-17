const { db } = require('../configs/firebase'); // Import your firebase setup

exports.getAverageViolationSum = async (req, res) => {
    try {
        const snapshot = await db
            .collection('ethics-violations')
            .orderBy('time_added', 'desc')
            .limit(7) // Get the newest 7 days' results
            .get();

        // Check if any documents were returned
        if (snapshot.empty) {
            return res.status(404).send('No data found.');
        }

        // Sum up violation_sum values
        let totalSum = 0;
        let count = 0;

        snapshot.forEach(doc => {
            totalSum += doc.data().violation_sum;
            count += 1;
        });

        // Calculate the average
        const average = totalSum / count;

        // Return the result as JSON
        res.status(200).json({ average });
    } catch (error) {
        console.error('Error reading from Firestore:', error);
        res.status(500).send('Error reading from Firestore.');
    }
};


exports.getNewestSevenViolations = async (req, res) => {
    try {
        const snapshot = await db
            .collection('ethics-violations')
            .orderBy('time_added', 'desc')
            .limit(7) // To get the newest 7 days result
            .get();

        // Check if any documents were returned
        if (snapshot.empty) {
            return res.status(404).send('No data found.');
        }

        const numbers = [];
        snapshot.forEach(doc => {
            numbers.push({ id: doc.id, violation_sum: doc.data().violation_sum, time_added: doc.data().time_added });
        });

        res.status(200).json(numbers); // Return the newest document as a single object
    } catch (error) {
        console.error('Error reading from Firestore:', error);
        res.status(500).send('Error reading from Firestore.');
    }
};