# Sunanda_VI
Linear Regression Model to Predict stock Closing price

# Stock Prediction using Linear Regression

This project implements a linear regression model to predict the closing price of a stock based on several features:

- Open, High, Low prices  
- Volume  
- Momentum Index  
- Market Sentiment  
- Trend Strength  
- Oscillator Value  
- Index  

The model is trained on historical stock data and uses scaling for features.  
The trained model is saved as 'Closeprice_predicter.npz' and can be loaded to make predictions for new data.  

## How to Use

1. Clone the repo  
2. Open 'Closeprice_stocks_predictions.ipyn'in Jupyter  
3. Run all cells to load the model and make predictions  
4. Enter the required stock features when prompted
5. use the 'Closeprice_predictor.npz' saved under file CLOSEPRICE_MODEL.ipyn to predict the close price by providng suitable input

## Model Performance

- R² Score: 0.94  
- Mean Absolute Error (MAE): 14.39  
- Approximate Accuracy: 78%
