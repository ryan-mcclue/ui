#!/usr/bin/python3
# SPDX-License-Identifier: zlib-acknowledgement
import os
import sys

__import__('pysqlite3')
import sys
sys.modules['sqlite3'] = sys.modules.pop('pysqlite3')

import openai
from langchain.chains import ConversationalRetrievalChain, RetrievalQA
from langchain.chat_models import ChatOpenAI
from langchain.document_loaders import DirectoryLoader, TextLoader
from langchain.embeddings import OpenAIEmbeddings
from langchain.indexes import VectorstoreIndexCreator
from langchain.indexes.vectorstore import VectorStoreIndexWrapper
from langchain.llms import OpenAI
from langchain.vectorstores import Chroma
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain.vectorstores import FAISS

import constants

os.environ["OPENAI_API_KEY"] = constants.APIKEY

#
#
## Enable to save to disk & reuse the model (for repeated queries on the same data)
#PERSIST = False
#
#query = None
#if len(sys.argv) > 1:
#  query = sys.argv[1]
#
#if PERSIST and os.path.exists("persist"):
#  print("Reusing index...\n")
#  vectorstore = Chroma(persist_directory="persist", embedding_function=OpenAIEmbeddings())
#  index = VectorStoreIndexWrapper(vectorstore=vectorstore)
#else:
#  #loader = TextLoader("data/data.txt") # Use this line if you only need data.txt
#  loader = DirectoryLoader("/home/ryan/prog/personal/learn/root_basic")
#  if PERSIST:
#    index = VectorstoreIndexCreator(vectorstore_kwargs={"persist_directory":"persist"}).from_loaders([loader])
#  else:
#    index = VectorstoreIndexCreator().from_loaders([loader])
#
## TODO(Ryan): perhaps change model to a 'code-based' one?
#chain = ConversationalRetrievalChain.from_llm(
#  #llm=ChatOpenAI(model="gpt-3.5-turbo"),
#  llm=ChatOpenAI(model="gpt-3.5-turbo-instruct"),
#  retriever=index.vectorstore.as_retriever(search_kwargs={"k": 1}),
#)
#
## documents/chunks -> embeddings database
#
## map/summaries
## 
#
## https://www.youtube.com/watch?v=h0DHDp1FbmQ
#
##  Limit: 1000000 / min. Current: 411398 / min 
#
#chat_history = []
#while True:
#  if not query:
#    query = input("Prompt: ")
#  if query in ['quit', 'q', 'exit']:
#    sys.exit()
#  result = chain({"question": query, "chat_history": chat_history})
#  print(result['answer'])
#
#  chat_history.append((query, result['answer']))
#  query = None
#
loader = DirectoryLoader("/home/ryan/prog/personal/learn/root_basic/code/base", glob="**/*.[ch]")
data = loader.load()
# at this point, data is collection of documents. each document contains number of characters
# want to reduce
print(f"{len(data)} documents")
print(f"{len(data[0].page_content)} characters")
print(f"{len(data[1].page_content)} characters")
print(f"{len(data[10].page_content)} characters")

text_splitter = RecursiveCharacterTextSplitter(chunk_size=2000, chunk_overlap=0)
texts = text_splitter.split_documents(data)
print(f"{len(texts)} documents")
# now all under 2000 characters

# embeddeings are compress
embeddings = OpenAIEmbeddings(openai_api_key=constants.APIKEY)

# vector store stores embeddings
#docsearch = FAISS.from_documents(texts, embeddings)
docsearch = Chroma.from_documents(texts, embeddings)

# the chain_type of stuff is searching within our documents for the one that is most relevent to search, so only send this to openai
qa = RetrievalQA.from_chain_type(llm=OpenAI(), chain_type="stuff", retriever=docsearch.as_retriever())

query = "give example code demonstrating how to parse command line arguments using CmdLine. example arguments like --verbose, -v, --help, -h etc."
print(qa.run(query))
