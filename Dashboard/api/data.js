export default async function handler(req, res) {

  const username = process.env.AIO_USERNAME;
  const key = process.env.AIO_KEY;

  const feeds = ["gas", "humidity", "pressure", "temperature"];

  try {
    const result = {};

    for (const feed of feeds) {

      const response = await fetch(
        `https://io.adafruit.com/api/v2/${username}/feeds/${feed}/data?limit=50`,
        {
          headers: {
            "X-AIO-Key": key
          }
        }
      );

      if (!response.ok) {
        throw new Error(`Failed to fetch ${feed}`);
      }

      result[feed] = await response.json();
    }

    res.status(200).json(result);

  } catch (err) {
    console.error(err);
    res.status(500).json({
      error: err.message
    });
  }

}
