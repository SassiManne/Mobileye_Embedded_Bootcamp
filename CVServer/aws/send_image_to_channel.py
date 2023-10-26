import asyncio
from telegram import Bot
from telegram import InputFile
import sys
import datetime
import os

async def send_image_to_channel(image_path, filename, frameNumber):
    token = "6327124762:AAFuOsLYzJ-SZe4dhH6xLbFMmojD4q8KSMY"
    chat_id = "@grpc_1"
    bot = Bot(token=token)
    print (image_path+filename)

    current_date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    caption = f"Sent on {current_date} frame {frameNumber}"
    

    with open(filename, 'rb') as image_file:
        photo = InputFile(image_file)
        await bot.send_photo(chat_id=chat_id, photo=photo, caption=caption)

if __name__ == "__main__":
    asyncio.run(send_image_to_channel(sys.argv[0],sys.argv[1] , sys.argv[2]))
    os.remove(sys.argv[1])  # Delete the image file
