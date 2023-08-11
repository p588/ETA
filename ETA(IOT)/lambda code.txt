import json
import boto3

def getFirstRecordFromTable(table_name):
    dynamodb = boto3.resource('dynamodb')
    
    table = dynamodb.Table(table_name)
    return table.scan(Limit=1)['CurrentPeople']['stopNumber']
    

def lambda_handler(event, context):
    dynamodb = boto3.resource('dynamodb')
    
    table = dynamodb.Table('esp32_t2')
    CurrentPeople = table.query(KeyConditionExpression='device_id = :a', ExpressionAttributeValues={":a": "esp32"},ScanIndexForward=False)['CurrentPeople']['stopNumber']
    # load = getFirstRecordFromTable('load_data2')['g']
   
    stopNumber =table.query(KeyConditionExpression='device_id = :a', ExpressionAttributeValues={":a": "esp32"},ScanIndexForward=False)['CurrentPeople']['stopNumber']
    # distance = getFirstRecordFromTable('sonic_data')['cm']
    
    
    return {
        'statusCode': 200,
        'CurrentPeople': CurrentPeople,
        'stopNumber': stopNumber
    }