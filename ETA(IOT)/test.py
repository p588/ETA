import streamlit as st
import requests
import time

# Define the API URL
api_url = "https://yg88mibxvk.execute-api.us-east-1.amazonaws.com/test/transtion"

# Create a function to fetch data from the API
def fetch_data():
    try:
        response = requests.get(api_url)
        if response.status_code == 200:
            data = response.json()
            return data
        else:
            st.error(f"Error: {response.status_code}")
            return None
    except requests.exceptions.RequestException as e:
        st.error(f"Error: {e}")
        return None

st.title('Bus Occupancy and ETA')

# Add content to the left column
st.header('Current People in Bus')
current_people = st.empty()

# Add content to the right column
st.header('Stop Number')
stop_number = st.empty()

# Continuously update the values
while True:
    data = fetch_data()
    if data:
        current_people.subheader('Current People:')
        current_people.header(data.get('CurrentPeople'))

        stop_number.subheader('Stop Number:')
        stop_number.header(data.get('StopNumber'))

    time.sleep(2)  # Update every 2 seconds
